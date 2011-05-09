#include <iostream>
#include <boost/thread/barrier.hpp>

#include "sched/static_local.hpp"
#include "db/database.hpp"
#include "db/tuple.hpp"
#include "process/remote.hpp"
#include "vm/state.hpp"

using namespace boost;
using namespace std;
using namespace process;
using namespace vm;
using namespace db;
using namespace utils;

namespace sched
{

static vector<static_local*> others;
barrier* static_local::thread_barrier(NULL);
termination_barrier* static_local::term_barrier(NULL);

void
static_local::threads_synchronize(void)
{
   thread_barrier->wait();
}

void
static_local::assert_end(void) const
{
   assert(queue_nodes.empty());
   assert(process_state == PROCESS_INACTIVE);
   assert(term_barrier->all_finished());
}

void
static_local::assert_end_iteration(void) const
{
   assert(queue_nodes.empty());
   assert(process_state == PROCESS_INACTIVE);
   assert(term_barrier->all_finished());
}

void
static_local::make_active(void)
{
   assert(process_state == PROCESS_INACTIVE);
   
   term_barrier->is_active();
   
   process_state = PROCESS_ACTIVE;
   
#ifdef DEBUG_ACTIVE
   cout << "Active " << id << endl;
#endif
}

void
static_local::make_inactive(void)
{
   assert(process_state == PROCESS_ACTIVE);
   
   term_barrier->is_inactive();
   
   process_state = PROCESS_INACTIVE;
   
#ifdef DEBUG_ACTIVE
   cout << "Inactive: " << id << endl;
#endif
}

void
static_local::new_work(node *from, node *_to, const simple_tuple *tpl, const bool is_agg)
{
   (void)from;
   thread_node *to((thread_node*)_to);
    
   assert(to != NULL);
   assert(tpl != NULL);
   
   to->add_work(tpl, is_agg);
   
   if(!to->in_queue()) {
      mutex::scoped_lock lock(to->mtx);
      if(!to->in_queue())
         add_to_queue(to);
      // no need to put owner active, since we own this node
      // new_work was called for init or for self generation of
      // tuples (SEND a TO a)
      // the lock is needed in order to make sure
      // the node is not put multiple times on the queue
   }
   
   assert(to->in_queue());
}

void
static_local::new_work_other(sched::base *scheduler, node *node, const simple_tuple *stuple)
{
   assert(process_state == PROCESS_ACTIVE);
   assert(node != NULL);
   assert(stuple != NULL);
   assert(scheduler == NULL);
   
   thread_node *tnode((thread_node*)node);
   
   assert(tnode->get_owner() != NULL);
   
   tnode->add_work(stuple, false);
   
   if(!tnode->in_queue()) {
      mutex::scoped_lock lock(tnode->mtx);
      if(!tnode->in_queue()) {
         static_local *owner(tnode->get_owner());
         owner->add_to_queue(tnode);
         
         if(this != owner && 
            owner->process_state == PROCESS_INACTIVE)
         {
            mutex::scoped_lock lock2(owner->mutex);
            
            if(owner->process_state == PROCESS_INACTIVE)
               owner->make_active();
            assert(owner->process_state == PROCESS_ACTIVE);
         }
         
         assert(tnode->in_queue());
      }
   }
}

void
static_local::new_work_remote(remote *, const vm::process_id, message *)
{
   assert(0);
}

void
static_local::generate_aggs(void)
{
   const node::node_id first(remote::self->find_first_node(id));
   const node::node_id final(remote::self->find_last_node(id));
   database::map_nodes::iterator it(state::DATABASE->get_node_iterator(first));
   database::map_nodes::iterator end(state::DATABASE->get_node_iterator(final));

   for(; it != end; ++it)
   {
      node *no(it->second);
      simple_tuple_list ls(no->generate_aggs());

      for(simple_tuple_list::iterator it2(ls.begin());
         it2 != ls.end();
         ++it2)
      {
         new_work(NULL, no, *it2);
      }
   }
}

bool
static_local::busy_wait(void)
{
   bool turned_inactive(false);
   
   while(queue_nodes.empty()) {
      
      if(!turned_inactive) {
         mutex::scoped_lock l(mutex);
         if(queue_nodes.empty() && process_state == PROCESS_ACTIVE) {
            make_inactive();
            turned_inactive = true;
            if(term_barrier->all_finished())
               return false;
         } else if(process_state == PROCESS_INACTIVE && queue_nodes.empty()) {
            turned_inactive = true;
         }
      }
      
      if(term_barrier->all_finished()) {
         assert(process_state == PROCESS_INACTIVE);
         return false;
      }
   }
   
   if(process_state == PROCESS_INACTIVE) {
      mutex::scoped_lock l(mutex);
      if(process_state == PROCESS_INACTIVE)
         make_active();
   }
   
   assert(process_state == PROCESS_ACTIVE);
   assert(!queue_nodes.empty());
   
   return true;
}

bool
static_local::terminate_iteration(void)
{
   // this is needed since one thread can reach make_active
   // and thus other threads waiting for all_finished will fail
   // to get here
   threads_synchronize();

   assert(process_state == PROCESS_INACTIVE);

   generate_aggs();

   if(!queue_nodes.empty()) {
      make_active();
   }

#ifdef ASSERT_THREADS
   static boost::mutex local_mtx;
   static vector<size_t> total;

   local_mtx.lock();

   total.push_back(iteration);

   if(total.size() == state::NUM_THREADS) {
      for(size_t i(0); i < state::NUM_THREADS; ++i) {
         assert(total[i] == iteration);
      }

      total.clear();
   }

   local_mtx.unlock();
#endif

   // again, needed since we must wait if any thread
   // is set to active in the previous if
   threads_synchronize();

   return !term_barrier->all_finished();
}

void
static_local::finish_work(const work_unit& work)
{
   assert(current_node != NULL);
   assert(current_node->in_queue());
}

bool
static_local::check_if_current_useless(void)
{
   if(current_node->no_more_work()) {
      mutex::scoped_lock lock(current_node->mtx);
      
      if(current_node->no_more_work()) {
         current_node->set_in_queue(false);
         assert(!current_node->in_queue());
         current_node = NULL;
         return true;
      }
   }
   
   assert(!current_node->no_more_work());
   return false;
}

bool
static_local::set_next_node(void)
{
   if(current_node != NULL)
      check_if_current_useless();
   
   while (current_node == NULL) {   
      if(queue_nodes.empty()) {
         if(!busy_wait())
            return false;
      }
      
      assert(!queue_nodes.empty());
      
      current_node = queue_nodes.pop();
      
      assert(current_node != NULL);
      
      check_if_current_useless();
   }
   
   assert(current_node != NULL);
   return true;
}

bool
static_local::get_work(work_unit& work)
{  
   if(!set_next_node())
      return false;
      
   assert(current_node != NULL);
   assert(current_node->in_queue());
   assert(!current_node->no_more_work());
   
   node_work_unit unit(current_node->get_work());
   
   work.work_tpl = unit.work_tpl;
   work.agg = unit.agg;
   work.work_node = current_node;
   
   assert(work.work_node == current_node);
   
   return true;
}

void
static_local::end(void)
{
}

void
static_local::init(const size_t num_threads)
{
   predicate *init_pred(state::PROGRAM->get_init_predicate());
   
   database::map_nodes::iterator it(state::DATABASE->get_node_iterator(remote::self->find_first_node(id)));
   database::map_nodes::iterator end(state::DATABASE->get_node_iterator(remote::self->find_last_node(id)));
   
   for(; it != end; ++it)
   {
      thread_node *cur_node((thread_node*)it->second);
      
      cur_node->set_owner(this);
      
      new_work(NULL, cur_node, simple_tuple::create_new(new vm::tuple(init_pred)));
      
      assert(cur_node->in_queue());
      assert(!cur_node->no_more_work());
   }
   
   threads_synchronize();
}

static_local*
static_local::find_scheduler(const node::node_id id)
{
   return NULL;
}

static_local::static_local(const vm::process_id _id):
   base(_id),
   process_state(PROCESS_ACTIVE),
   current_node(NULL)
{
}

static_local::~static_local(void)
{
}

void
static_local::init_barriers(const size_t num_threads)
{
   thread_barrier = new barrier(num_threads);
   term_barrier = new termination_barrier(num_threads);
}
   
vector<static_local*>&
static_local::start(const size_t num_threads)
{
   init_barriers(num_threads);
   others.resize(num_threads);
   
   for(process_id i(0); i < num_threads; ++i)
      others[i] = new static_local(i);
      
   return others;
}
   
}