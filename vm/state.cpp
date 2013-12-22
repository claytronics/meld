
#include <iostream>
#include "vm/state.hpp"
#include "process/machine.hpp"
#include "vm/exec.hpp"
#include "debug/debug_handler.hpp"
#include "api/api.hpp"

using namespace vm;
using namespace db;
using namespace process;
using namespace std;
using namespace runtime;
using namespace utils;

//#define DEBUG_RULES

namespace vm
{

#ifdef USE_UI
bool state::UI = false;
#endif
#ifdef USE_SIM
bool state::SIM = false;
#endif

void
state::purge_runtime_objects(void)
{
#define PURGE_OBJ(TYPE) \
   for(list<TYPE*>::iterator it(free_ ## TYPE.begin()); it != free_ ## TYPE .end(); ++it) { \
      TYPE *x(*it); \
      assert(x != NULL); \
      x->dec_refs(); \
   } \
   free_ ## TYPE .clear()

   PURGE_OBJ(cons);
	PURGE_OBJ(rstring);
   PURGE_OBJ(struct1);
	
#undef PURGE_OBJ
}

void
state::cleanup(void)
{
   purge_runtime_objects();
   removed.clear();
}

void
state::copy_reg2const(const reg_num& reg_from, const const_id& cid)
{
  vm::All->set_const(cid, regs[reg_from]);
  switch(vm::All->PROGRAM->get_const_type(cid)->get_type()) {
  case FIELD_LIST:
    runtime::cons::inc_refs(vm::All->get_const_cons(cid)); break;
  case FIELD_STRING:
    vm::All->get_const_string(cid)->inc_refs(); break;
  default: break;
  }
}

void
state::setup(vm::tuple *tpl, db::node *n, const derivation_count count, const depth_t depth)
{
   this->use_local_tuples = false;
   this->node = n;
   this->count = count;
   if(tpl != NULL) {
      if(tpl->is_cycle())
         this->depth = depth + 1;
      else
         this->depth = 0;
   } else {
      this->depth = 0;
   }
	if(tpl != NULL)
   	this->is_linear = tpl->is_linear();
	else
		this->is_linear = false;
   for(size_t i(0); i < NUM_REGS; ++i) {
      this->saved_leaves[i] = NULL;
      this->is_leaf[i] = false;
   }
#ifdef CORE_STATISTICS
   this->stat.start_matching();
#endif
}

void
state::mark_active_rules(void)
{
	for(rule_matcher::rule_iterator it(store->matcher.begin_active_rules()),
		end(store->matcher.end_active_rules());
		it != end;
		it++)
	{
		rule_id rid(*it);
		if(!store->rules[rid]) {
			// we need check if at least one predicate was activated in this loop
			vm::rule *rule(All->PROGRAM->get_rule(rid));
         bool flag = false;
         for(rule::predicate_iterator jt(rule->begin_predicates()), endj(rule->end_predicates());
               jt != endj;
               jt++)
         {
            const predicate *pred(*jt);
            if(store->predicates[pred->get_id()]) {
               flag = true;
               break;
            }
         }
			if(flag) {
				store->rules[rid] = true;
				heap_priority pr;
				pr.int_priority = (int)rid;
				rule_queue.insert(rid, pr);
			}
		}
	}
	
	for(rule_matcher::rule_iterator it(store->matcher.begin_dropped_rules()),
		end(store->matcher.end_dropped_rules());
		it != end;
		it++)
	{
		rule_id rid(*it);
		if(store->rules[rid]) {
			store->rules[rid] = false;
         heap_priority pr;
         pr.int_priority = (int)rid;
			rule_queue.remove(rid, pr);
		}
	}

   store->matcher.clear_dropped_rules();
}

bool
state::add_fact_to_node(vm::tuple *tpl, const vm::derivation_count count, const vm::depth_t depth)
{
#ifdef CORE_STATISTICS
   execution_time::scope s(stat.db_insertion_time_predicate[tpl->get_predicate_id()]);
#endif

	return node->add_tuple(tpl, count, depth);
}

static inline bool
tuple_for_assertion(db::simple_tuple *stpl)
{
   return ((stpl->get_tuple())->is_aggregate() && stpl->is_aggregate())
      || !stpl->get_tuple()->is_aggregate();
}

db::simple_tuple*
state::search_for_negative_tuple_partial_agg(db::simple_tuple *stpl)
{
   vm::tuple *tpl(stpl->get_tuple());

   assert(!stpl->is_aggregate());

   for(db::simple_tuple_list::iterator it(store->persistent_tuples.begin()),
         end(store->persistent_tuples.end());
         it != end; ++it)
   {
      db::simple_tuple *stpl2(*it);
      vm::tuple *tpl2(stpl2->get_tuple());


      if(tpl2->is_aggregate() && !stpl2->is_aggregate() &&
            stpl2->get_count() == -1 && *tpl2 == *tpl)
      {
         store->persistent_tuples.erase(it);
         return stpl2;
      }
   }

   return NULL;
}

db::simple_tuple*
state::search_for_negative_tuple_normal(db::simple_tuple *stpl)
{
   vm::tuple *tpl(stpl->get_tuple());

   assert(!tpl->is_aggregate());
   assert(!stpl->is_aggregate());

   for(db::simple_tuple_list::iterator it(store->persistent_tuples.begin()),
         end(store->persistent_tuples.end());
         it != end; ++it)
   {

      db::simple_tuple *stpl2(*it);
      vm::tuple *tpl2(stpl2->get_tuple());

      if(!tpl2->is_aggregate() && stpl2->get_count() == -1 && *tpl2 == *tpl)
      {
         store->persistent_tuples.erase(it);
         return stpl2;
      }
   }

   return NULL;
}

db::simple_tuple*
state::search_for_negative_tuple_full_agg(db::simple_tuple *stpl)
{
   vm::tuple *tpl(stpl->get_tuple());

   for(db::simple_tuple_list::iterator it(store->persistent_tuples.begin()),
         end(store->persistent_tuples.end());
         it != end; ++it)
   {


      db::simple_tuple *stpl2(*it);
      vm::tuple *tpl2(stpl2->get_tuple());

      if(tpl2->is_aggregate() && stpl2->is_aggregate() && stpl2->get_count() == -1 && *tpl2 == *tpl)
      {
         store->persistent_tuples.erase(it);
         return stpl2;
      }
   }

   return NULL;
}


void
state::delete_leaves(void)
{
   while(!leaves_for_deletion.empty()) {
      pair<vm::predicate*, db::tuple_trie_leaf*> p(leaves_for_deletion.front());
      vm::predicate* pred(p.first);

#ifdef CORE_STATISTICS
   	execution_time::scope s(stat.db_deletion_time_predicate[pred->get_id()]);
#endif
      db::tuple_trie_leaf *leaf(p.second);
      
      leaves_for_deletion.pop_front();
      
      leaf->reset_ref_use();
      node->delete_by_leaf(pred, leaf, 0);
   }
      
	assert(leaves_for_deletion.empty());
}

bool
state::do_persistent_tuples(void)
{
   while(!store->persistent_tuples.empty()) {
#ifdef USE_SIM
      if(check_instruction_limit()) {
         return false;
      }
#endif
      db::simple_tuple *stpl(store->persistent_tuples.front());
      vm::tuple *tpl(stpl->get_tuple());

      store->persistent_tuples.pop_front();

      if(tpl->is_persistent()) {
         // XXX crashes when calling wipeout below
         if(stpl->get_count() == 1 && (tpl->is_aggregate() && !stpl->is_aggregate())) {
            db::simple_tuple *stpl2(search_for_negative_tuple_partial_agg(stpl));
            if(stpl2) {
               assert(stpl != stpl2);
               //assert(stpl2->get_tuple() != stpl->get_tuple());
               //assert(stpl2->get_predicate() == stpl->get_predicate());
               //simple_tuple::wipeout(stpl);
               //simple_tuple::wipeout(stpl2);
               continue;
            }
         }

         if(stpl->get_count() == 1 && (tpl->is_aggregate() && stpl->is_aggregate())) {
            db::simple_tuple *stpl2(search_for_negative_tuple_full_agg(stpl));
            if(stpl2) {
               assert(stpl != stpl2);
               /*if(stpl2->get_tuple() == stpl->get_tuple()) {
                  cout << "fail " << *(stpl2->get_tuple()) << endl;
               }
               assert(stpl2->get_tuple() != stpl->get_tuple());
               assert(stpl2->get_predicate() == stpl->get_predicate());*/
               //simple_tuple::wipeout(stpl);
               //simple_tuple::wipeout(stpl2);
               continue;
            }
         }

         if(stpl->get_count() == 1 && !tpl->is_aggregate()) {
            db::simple_tuple *stpl2(search_for_negative_tuple_normal(stpl));
            if(stpl2) {
               //simple_tuple::wipeout(stpl);
               //simple_tuple::wipeout(stpl2);
               continue;
            }
         }
      }

      if(tuple_for_assertion(stpl)) {
#ifdef DEBUG_RULES
      cout << ">>>>>>>>>>>>> Running process for " << node->get_id() << " " << *stpl << " (" << stpl->get_depth() << ")" << endl;
#endif
         process_persistent_tuple(stpl, tpl);
      } else {
         // aggregate
#ifdef DEBUG_RULES
      cout << ">>>>>>>>>>>>> Adding aggregate " << node->get_id() << " " << *stpl << " (" << stpl->get_depth() << ")" << endl;
#endif
         add_to_aggregate(stpl);
      }
   }
   store->persistent_tuples.clear();
   delete_leaves();
   
   return true;
}

void
state::process_action_tuples(void)
{
   store->action_tuples.splice(store->action_tuples.end(), store->incoming_action_tuples);
   for(db::simple_tuple_list::iterator it(store->action_tuples.begin()),
         end(store->action_tuples.end());
         it != end;
         ++it)
   {
      db::simple_tuple *stpl(*it);
      vm::tuple *tpl(stpl->get_tuple());
      All->MACHINE->run_action(sched, node, tpl);
      delete stpl;
   }
   store->action_tuples.clear();
}

void
state::process_incoming_tuples(void)
{
   for(size_t i(0); i < store->num_lists; ++i) {
      db::intrusive_list<vm::tuple> *ls(store->get_incoming(i));
      for(db::intrusive_list<vm::tuple>::iterator it(ls->begin()), end(ls->end());
         it != end; ++it)
      {
         store->register_tuple_fact(*it, 1);
      }
      db::intrusive_list<vm::tuple> *inc(node->db.get_list(i));
      if(!ls->empty())
         inc->splice(*ls);
   }
   if(!store->incoming_persistent_tuples.empty())
      store->persistent_tuples.splice(store->persistent_tuples.end(), store->incoming_persistent_tuples);
}

void
state::print_local_tuples(ostream& cout)
{
#if 0 /// XXX
  if (generated_tuples.empty())
    cout << "(empty)" << endl;

  for(db::simple_tuple_list::iterator it(local_tuples.begin());
      it != local_tuples.end();
      ++it)
    {
      simple_tuple *stpl(*it);
      cout << *stpl << endl;
    }
#endif
}


void
state::print_generated_tuples(ostream& cout)
{
#if 0 /// XXX
  if (generated_tuples.empty())
    cout << "(empty)" << endl;

  for(db::simple_tuple_list::iterator it(generated_tuples.begin());
      it != generated_tuples.end();
      ++it)
    {
      simple_tuple *stpl(*it);
      cout << *stpl << endl;
    }
#endif
}

void
state::add_to_aggregate(db::simple_tuple *stpl)
{
   vm::tuple *tpl(stpl->get_tuple());
   const predicate *pred(tpl->get_predicate());
   vm::derivation_count count(stpl->get_count());
   agg_configuration *agg(NULL);

   if(count < 0) {
      agg = node->remove_agg_tuple(tpl, -count, stpl->get_depth());
   } else {
      agg = node->add_agg_tuple(tpl, count, stpl->get_depth());
   }

   simple_tuple_list list;


   agg->generate(pred->get_aggregate_type(), pred->get_aggregate_field(), list);

   for(simple_tuple_list::iterator it(list.begin()); it != list.end(); ++it) {
      simple_tuple *stpl(*it);
      stpl->set_as_aggregate();
      store->persistent_tuples.push_back(stpl);
   }
}

void
state::process_persistent_tuple(db::simple_tuple *stpl, vm::tuple *tpl)
{
   // persistent tuples are marked inside this loop
   if(stpl->get_count() > 0) {
      bool is_new;

      if(tpl->is_reused()) {
         is_new = true;
      } else {
         is_new = add_fact_to_node(tpl, stpl->get_count(), stpl->get_depth());
      }

      if(is_new) {
         setup(tpl, node, stpl->get_count(), stpl->get_depth());
         use_local_tuples = false;
         persistent_only = true;
         execute_bytecode(vm::All->PROGRAM->get_predicate_bytecode(tpl->get_predicate_id()), *this, tpl);
      }

      if(tpl->is_reused()) {
         node->add_linear_fact(tpl);
      } else {
         store->matcher.register_tuple(tpl, stpl->get_count(), is_new);

         if(is_new) {
            store->mark(tpl->get_predicate());
         } else {
            vm::tuple::destroy(tpl);
         }
      }

      delete stpl;
   } else {
		if(tpl->is_reused()) {
			setup(tpl, node, stpl->get_count(), stpl->get_depth());
			persistent_only = true;
			use_local_tuples = false;
			execute_bytecode(vm::All->PROGRAM->get_predicate_bytecode(tpl->get_predicate_id()), *this, tpl);
         delete stpl;
		} else {
      	node::delete_info deleter(node->delete_tuple(tpl, -stpl->get_count(), stpl->get_depth()));

         if(!deleter.is_valid()) {
            // do nothing... it does not exist
         } else if(deleter.to_delete()) { // to be removed
            store->matcher.deregister_tuple(tpl, -stpl->get_count());
         	setup(tpl, node, stpl->get_count(), stpl->get_depth());
         	persistent_only = true;
         	use_local_tuples = false;
         	execute_bytecode(vm::All->PROGRAM->get_predicate_bytecode(tpl->get_predicate_id()), *this, tpl);
         	deleter();

		debugger::runBreakPoint("factRet",
			      "Fact has been removed from database",
                                (char*)tpl->pred_name().c_str(),
			      (int)node->get_translated_id());


      	} else if(tpl->is_cycle()) {
            store->matcher.deregister_tuple(tpl, -stpl->get_count());
            depth_counter *dc(deleter.get_depth_counter());
            assert(dc != NULL);

            if(dc->get_count(stpl->get_depth()) == 0) {
	      //vm::derivation_count deleted(deleter.delete_depths_above(stpl->get_depth()));
	      // if you need to access this for debugging purposes,
	      // uncomment the above line and comment out the
	      // following line.
	      deleter.delete_depths_above(stpl->get_depth());
               if(deleter.to_delete()) {
                  setup(tpl, node, stpl->get_count(), stpl->get_depth());
                  persistent_only = true;
                  use_local_tuples = false;
                  execute_bytecode(vm::All->PROGRAM->get_predicate_bytecode(tpl->get_predicate_id()), *this, tpl);
                  deleter();
         debugger::runBreakPoint("factRet","Fact has been retracted",
         (char*)tpl->pred_name().c_str(),
         (int)node->get_translated_id());
               }
            }
         } else {
            debugger::runBreakPoint("factRet","Fact has been retracted",
                  (char*)tpl->pred_name().c_str(),
                  (int)node->get_translated_id());
            store->matcher.deregister_tuple(tpl, -stpl->get_count());
            vm::tuple::destroy(tpl);
         }
         delete stpl;

		}

   }
}

void
state::run_node(db::node *no)
{
   bool aborted(false);

	this->node = no;
	
#ifdef DEBUG_RULES
   cout << "Node " << node->get_id() << " (is " << node->get_translated_id() << ")" << endl;
#endif

   store = &(no->store);
   lists = &(no->db);

	{
#ifdef CORE_STATISTICS
		execution_time::scope s(stat.core_engine_time);
#endif
      no->lock();
      process_action_tuples();
		process_incoming_tuples();
      no->unprocessed_facts = false;
      no->unlock();
	}
	
   if(do_persistent_tuples()) {
#ifdef CORE_STATISTICS
		execution_time::scope s(stat.core_engine_time);
#endif
      mark_active_rules();
   } else
      // if using the simulator, we check if we exhausted the available time to run
      aborted = true;

	while(!rule_queue.empty() && !aborted) {

		rule_id rule(rule_queue.pop());


#ifdef DEBUG_RULES
		cout << node->get_id() << ":Run rule " << vm::All->PROGRAM->get_rule(rule)->get_string() << endl;
#endif



		/* delete rule and every check */
		{
#ifdef CORE_STATISTICS
			execution_time::scope s(stat.core_engine_time);
#endif
			store->rules[rule] = false;
         store->clear_predicates();
		}
		
		setup(NULL, node, 1, 0);

		use_local_tuples = true;
      persistent_only = false;
		execute_rule(rule, *this);

      delete_leaves();
      //node->assert_tries();
#ifdef USE_SIM
      if(sim_instr_use && !check_instruction_limit()) {
         // gather new tuples
         db::simple_tuple_list new_tuples;
         sched::sim_node *snode(dynamic_cast<sched::sim_node*>(node));
         snode->get_tuples_until_timestamp(new_tuples, sim_instr_limit);
         local_tuples.splice(local_tuples.end(), new_tuples);
      }
#endif
      /* move from generated tuples to local_tuples */
      for(size_t i(0); i < store->num_lists; ++i) {
         db::intrusive_list<vm::tuple> *gen(store->get_generated(i));
         db::intrusive_list<vm::tuple> *ls(node->db.get_list(i));
         if(!gen->empty())
            ls->splice(*gen);
      }
      if(!do_persistent_tuples()) {
         aborted = true;
         break;
      }
		{
#ifdef CORE_STATISTICS
			execution_time::scope s(stat.core_engine_time);
#endif
			mark_active_rules();
         api::regularPollAndProcess(sched);
		}
	}

#if 0
   // push remaining tuples into node
   for(size_t i(0); i < store->num_lists; ++i) {
      db::simple_tuple_list *ls(store->get_list(i));
      for(db::simple_tuple_list::iterator it(ls->begin()), end(ls->end());
         it != end; ++it)
      {
         simple_tuple *stpl(*it);
         vm::tuple *tpl(stpl->get_tuple());
		
         if(stpl->can_be_consumed()) {
            if(aborted) {
#ifdef USE_SIM
               sched::sim_node *snode(dynamic_cast<sched::sim_node*>(node));
               snode->pending.push(stpl);
#else
               assert(false);
#endif
            } else {
               add_fact_to_node(tpl, stpl->get_count(), stpl->get_depth());
               delete stpl;
            }
         } else {
            db::simple_tuple::wipeout(stpl);
         }
      }
      ls->clear();
	}
#endif

   rule_queue.clear();

#ifdef USE_SIM
   // store any remaining persistent tuples
   sched::sim_node *snode(dynamic_cast<sched::sim_node*>(node));
   for(simple_tuple_list::iterator it(store->persistent_tuples->begin()), end(store->persistent_tuples->end());
         it != end; ++it)
   {
      assert(aborted);
      db::simple_tuple *stpl(*it);
      snode->pending.push(stpl);
   }
#endif
	
   store->persistent_tuples.clear();
#ifdef USE_SIM
   if(sim_instr_use && sim_instr_counter < sim_instr_limit)
      ++sim_instr_counter;
#endif
}

state::state(sched::base *_sched):
   sched(_sched)
#ifdef DEBUG_MODE
   , print_instrs(false)
#endif
#ifdef CORE_STATISTICS
   , stat(vm::All)
#endif
{
	rule_queue.set_type(HEAP_INT_ASC);
}

state::state(void):
   sched(NULL)
#ifdef DEBUG_MODE
   , print_instrs(false)
#endif
   , persistent_only(false)
#ifdef CORE_STATISTICS
   , stat(vm::All)
#endif
{
#ifdef USE_SIM
   sim_instr_use = false;
#endif
}

state::~state(void)
{
#ifdef CORE_STATISTICS
	if(sched != NULL) {
      stat.print(cout, all);
	}
#endif
	assert(rule_queue.empty());
}

}


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
