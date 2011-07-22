
#include "sched/local/threads_direct.hpp"
#include "vm/state.hpp"
#include "db/database.hpp"
#include "process/remote.hpp"
#include "utils/utils.hpp"
#include "sched/common.hpp"
#include "sched/thread/assert.hpp"

using namespace vm;
using namespace utils;
using namespace db;
using namespace boost;
using namespace std;
using namespace process;

namespace sched
{

void
direct_local::assert_end(void) const
{
#ifdef MARK_OWNED_NODES
   for(node_set::iterator it(nodes->begin()); it != nodes->end(); ++it) {
      thread_node *node((thread_node*)*it);
      node->assert_end();
   }
#else
   assert_static_nodes_end(id);
#endif
}

void
direct_local::assert_end_iteration(void) const
{
#ifdef MARK_OWNED_NODES
   for(node_set::iterator it(nodes->begin()); it != nodes->end(); ++it) {
      thread_node *node((thread_node*)*it);
      node->assert_end_iteration();
   }
#else
	 assert_static_nodes_end_iteration(id);
#endif
}

#ifdef MARK_OWNED_NODES
void
direct_local::add_node(node *node)
{
   assert(nodes != NULL);
   assert(node != NULL);
   
   spinlock::scoped_lock l(*nodes_mutex);

   nodes->insert(node);
}

void
direct_local::remove_node(node *node)
{
   assert(nodes != NULL);
   assert(node != NULL);
   
   spinlock::scoped_lock l(*nodes_mutex);

   nodes->erase(node);
}
#endif

void
direct_local::end(void)
{
}

void
direct_local::new_agg(work& new_work)
{
   thread_node *to(dynamic_cast<thread_node*>(new_work.get_node()));
   
   assert_thread_push_work();
   
   node_work node_new_work(new_work);
   
   to->add_work(node_new_work);
   
   if(!to->in_queue()) {
      to->set_in_queue(true);
#ifdef MARK_OWNED_NODES
      add_to_queue(to);
      assert(this == to->get_owner());
#else
      // note the 'get_owner'
		direct_local *owner(dynamic_cast<direct_local*>(to->get_owner()));
      owner->add_to_queue(to);
#endif
   }
}

void
direct_local::new_work(const node *, work& new_work)
{
   thread_node *to(dynamic_cast<thread_node*>(new_work.get_node()));
   
   assert_thread_push_work();
   
   node_work node_new_work(new_work);
   
   to->add_work(node_new_work);
   
   if(!to->in_queue() && to->has_work()) {
      spinlock::scoped_lock l(to->spin);
      if(!to->in_queue() && to->has_work()) {
         // note that this node can now have a different owner
         direct_local *owner(dynamic_cast<direct_local*>(to->get_owner()));
         owner->add_to_queue(to);
         to->set_in_queue(true);
      }
   }
}

void
direct_local::new_work_other(sched::base *, work& new_work)
{
   assert(is_active());
   
   thread_node *tnode(dynamic_cast<thread_node*>(new_work.get_node()));
   
   assert(tnode->get_owner() != NULL);
   
   assert_thread_push_work();
   
   node_work node_new_work(new_work);
   tnode->add_work(node_new_work);
   
	spinlock::scoped_lock l(tnode->spin);
   if(!tnode->in_queue() && tnode->has_work()) {
		direct_local *owner(dynamic_cast<direct_local*>(tnode->get_owner()));
		
		tnode->set_in_queue(true);
		owner->add_to_queue(tnode);

      if(this != owner) {
         spinlock::scoped_lock l2(owner->lock);
         
         if(owner->is_inactive() && owner->has_work())
         {
            owner->set_active();
            assert(owner->is_active());
         }
      } else {
         assert(is_active());
      }
         
      assert(tnode->in_queue());
   }
   
#ifdef INSTRUMENTATION
   sent_facts++;
#endif
}

direct_local*
direct_local::select_steal_target(void) const
{
   size_t idx(random_unsigned(state::NUM_THREADS));
   
   while(ALL_THREADS[idx] == this)
      idx = random_unsigned(state::NUM_THREADS);
   
   return dynamic_cast<direct_local*>(ALL_THREADS[idx]);
}

static inline size_t
find_max_steal_attempts(void)
{
   return state::NUM_THREADS + state::NUM_THREADS/2;
}

static inline size_t
get_max_send_nodes_per_time(void)
{
   return min((size_t)10, max((size_t)2, state::NUM_NODES_PER_PROCESS / STEAL_NODES_FACTOR));
}

void
direct_local::try_to_steal(void)
{
   if(state::NUM_THREADS == 1)
      return;
      
   size_t total(get_max_send_nodes_per_time());
      
   for(size_t attempt(0); attempt < find_max_steal_attempts(); ++attempt) {
      direct_local *target(select_steal_target());
      
      if(target->is_active()) {
         thread_node *new_node(NULL);
         assert(target != NULL);
         if(target->queue_nodes.pop(new_node)) {
            change_node(new_node, target);
            if(--total == 0)
               return;
         }
      }
   }
}

bool
direct_local::busy_wait(void)
{
   try_to_steal();
   
   while(!has_work()) {
      BUSY_LOOP_MAKE_INACTIVE()
      BUSY_LOOP_CHECK_TERMINATION_THREADS()
   }
   
   set_active_if_inactive();
   
   assert(is_active());
   
   return true;
}

void
direct_local::change_node(thread_node *node, direct_local *from)
{
   assert(is_active());
   assert(node != NULL);
   assert(from != NULL);
   assert(node != current_node);
   assert(node->has_work());
   assert(node->get_owner() == from);
   assert(node->in_queue()); // not in a real queue but marked as being in a queue
   
   // change ownership
   
#ifdef MARK_OWNED_NODES
   from->remove_node(node);
   add_node(node);
#endif
   
   {
      utils::spinlock l(node->spin);

      node->set_owner(dynamic_cast<base*>(this));
      assert(node->get_owner() == this);
   }

	add_to_queue(node);

#ifdef INSTRUMENTATION
   stealed_nodes++;
#endif
}
   
void
direct_local::init(const size_t)
{
#ifdef MARK_OWNED_NODES
   nodes_mutex = new spinlock();
   nodes = new node_set();
#endif

   database::map_nodes::iterator it(state::DATABASE->get_node_iterator(remote::self->find_first_node(id)));
   database::map_nodes::iterator end(state::DATABASE->get_node_iterator(remote::self->find_last_node(id)));
   
   for(; it != end; ++it)
   {
      thread_node *cur_node((thread_node*)it->second);
     
      cur_node->set_owner(this);
      
#ifdef MARK_OWNED_NODES
      nodes->insert(cur_node);
#endif   
   
      init_node(cur_node);
      
      assert(cur_node->in_queue());
      assert(cur_node->has_work());
   }
   
   threads_synchronize();
}

void
direct_local::generate_aggs(void)
{
#ifdef MARK_OWNED_NODES
   for(node_set::iterator it(nodes->begin()); it != nodes->end(); ++it) {
      node *node(*it);
      assert(((thread_node*)node)->get_owner() == this);
      node_iteration(node);
   }
#else
   iterate_static_nodes(id);
#endif
}

bool
direct_local::terminate_iteration(void)
{
   // this is needed since one thread can reach set_active
   // and thus other threads waiting for all_finished will fail
   // to get here
   
   assert_thread_end_iteration();
   
   threads_synchronize();

   assert(is_inactive());

   generate_aggs();

#ifndef MARK_OWNED_NODES
	 threads_synchronize();
#endif

   if(has_work())
      set_active();

   assert_thread_iteration(iteration);

   // again, needed since we must wait if any thread
   // is set to active in the previous if
   threads_synchronize();

   const bool ret(!all_threads_finished());
   
   threads_synchronize();

   if(!all_threads_finished())
      set_active_if_inactive();

   threads_synchronize();
   
   return ret;
}

bool
direct_local::check_if_current_useless(void)
{
   if(!current_node->has_work()) {
      spinlock::scoped_lock lock(current_node->spin);
      
      if(!current_node->has_work()) {
         current_node->set_in_queue(false);
         assert(!current_node->in_queue());
         current_node = NULL;
         return true;
      }
   }
   
   assert(current_node->has_work());
   return false;
}

bool
direct_local::set_next_node(void)
{
   if(current_node != NULL)
      check_if_current_useless();
   
   while (current_node == NULL) {   
      if(!has_work()) {
         if(!busy_wait())
            return false;
      }
      
      if(!queue_nodes.pop(current_node)) {
         assert(current_node == NULL);
         continue;
      }
      
      assert(current_node->in_queue());
      assert(current_node != NULL);
      assert(current_node->has_work());
   }
   
   assert(current_node != NULL);
   return true;
}

bool
direct_local::get_work(work& new_work)
{  
   if(!set_next_node())
      return false;
   
   assert(current_node != NULL);
   assert(current_node->in_queue());
   assert(current_node->has_work());
   
   node_work unit(current_node->get_work());
   
   new_work.copy_from_node(current_node, unit);
   
   assert(new_work.get_node() == current_node);
   
   assert_thread_pop_work();
   
   return true;
}

void
direct_local::write_slice(stat::slice& sl) const
{
#ifdef INSTRUMENTATION
   base::write_slice(sl);
   threaded::write_slice(sl);
   sl.work_queue = queue_nodes.size();
   sl.stealed_nodes = stealed_nodes;
   stealed_nodes = 0;
#else
   (void)sl;
#endif
}

direct_local*
direct_local::find_scheduler(const node *n)
{
   const thread_node *tn(dynamic_cast<const thread_node*>(n));
   
   if(tn->get_owner() == this)
      return this;
   else
      return NULL;
}

direct_local::direct_local(const process_id id):
   base(id),
   current_node(NULL)
#ifdef MARK_OWNED_NODES
   , nodes(NULL),
   nodes_mutex(NULL)
#endif
#ifdef INSTRUMENTATION
   , stealed_nodes(0)
#endif
{
}
   
direct_local::~direct_local(void)
{
#ifdef MARK_OWNED_NODES
   assert(nodes != NULL);
   assert(nodes_mutex != NULL);
   
   delete nodes;
   delete nodes_mutex;
#endif
}

vector<sched::base*>&
direct_local::start(const size_t num_threads)
{
   init_barriers(num_threads);
   
   for(process_id i(0); i < num_threads; ++i)
      add_thread(new direct_local(i));
      
   return ALL_THREADS;
}
   
}
