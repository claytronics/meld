
#include <algorithm>

#include "sched/base.hpp"
#include "process/work.hpp"
#include "db/tuple.hpp"
#include "db/neighbor_agg_configuration.hpp"
#include "vm/exec.hpp"
#include "process/machine.hpp"
#include "api/api.hpp"
#include "debug/debug_handler.hpp"


using namespace std;
using namespace boost;
using namespace vm;
using namespace process;
using namespace db;

namespace sched
{

  static bool init(void);

  pthread_key_t sched_key;
  static bool started(init());

  static void
  cleanup_sched_key(void)
  {
   pthread_key_delete(sched_key);
 }

 static bool init(void)
 {
   int ret(pthread_key_create(&sched_key, NULL));
   assert(ret == 0);
   atexit(cleanup_sched_key);
   return true;
 }

 void
 base::do_tuple_add(node *node, vm::tuple *tuple, const ref_count count)
 {
   if(tuple->is_linear()) {
    state.setup(tuple, node, count);
    const byte_code code(state.all->PROGRAM->get_predicate_bytecode(tuple->get_predicate_id()));
    const execution_return ret(execute_bytecode(code, state));

    if(ret == EXECUTION_CONSUMED) {
     delete tuple;
   } else {
     node->add_tuple(tuple, count);
   }
 } else {
  const bool is_new(node->add_tuple(tuple, count));

  if(is_new) {
         // set vm state
   state.setup(tuple, node, count);
   byte_code code(state.all->PROGRAM->get_predicate_bytecode(tuple->get_predicate_id()));
   execute_bytecode(code, state);
 } else
 delete tuple;
}
}

void
base::do_agg_tuple_add(node *node, vm::tuple *tuple, const ref_count count)
{
   const predicate *pred(tuple->get_predicate()); // get predicate here since tuple can be deleted!
   agg_configuration *conf(node->add_agg_tuple(tuple, count));
   const aggregate_safeness safeness(pred->get_agg_safeness());

   switch(safeness) {
    case AGG_UNSAFE: return;
    case AGG_IMMEDIATE: {
     simple_tuple_list list;

     conf->generate(pred->get_aggregate_type(), pred->get_aggregate_field(), list);

     for(simple_tuple_list::iterator it(list.begin()); it != list.end(); ++it) {
      simple_tuple *tpl(*it);
      new_work_agg(node, tpl);
    }
    return;
  }
  break;
#if 0
      case AGG_LOCALLY_GENERATED: {
         const strat_level level(pred->get_agg_strat_level());

         if(node->get_local_strat_level() < level) {
            return;
         }
      }
      break;
#endif
      case AGG_NEIGHBORHOOD:
      case AGG_NEIGHBORHOOD_AND_SELF: {
       const neighbor_agg_configuration *neighbor_conf(dynamic_cast<neighbor_agg_configuration*>(conf));

       if(!neighbor_conf->all_present()) {
        return;
      }
    }
    break;
    default: return;
  }

  simple_tuple_list list;
  conf->generate(pred->get_aggregate_type(), pred->get_aggregate_field(), list);

  for(simple_tuple_list::iterator it(list.begin()); it != list.end(); ++it) {
    simple_tuple *tpl(*it);

    assert(tpl->get_count() > 0);
    new_work_agg(node, tpl);
  }
}

void
base::do_work(db::node *node)
{
 state.run_node(node);

}


// Loop of execution
void
base::do_loop(void)
{
  db::node *node(NULL);

  while(true) {
      api::serializeBeginExec();

      while ((node = get_work())) {
          // Current processor has local work, process work
          do_work(node);
          finish_work(node);
      }

      bool hasWork = api::pollAndProcess(this, state.all);
      bool ensembleFinished = false;
      if (!hasWork)
          ensembleFinished = api::ensembleFinished(this);
      api::serializeEndExec();
      if (ensembleFinished)
          break;
  }
}

void
base::loop(void)
{
   // start process pool
   // multi-thread memory manager / allocator
 mem::ensure_pool();

   // Init is based on scheduler type
 init(state.all->NUM_THREADS);

 do_loop();

 assert_end();
 end();
   // cout << "DONE " << id << endl;
}

base*
base::get_scheduler(void)
{
 sched::base *s((sched::base*)pthread_getspecific(sched_key));
 return s;
}

void
base::start(void)
{
 pthread_setspecific(sched_key, this);
 if(id == 0) {
       // Main thread
  thread = new boost::thread();
  loop();
}
}

base::~base(void)
{
	delete thread;
}

base::base(const vm::process_id _id, vm::all *_all):
id(_id),
thread(NULL),
state(this, _all),
iteration(0)
#ifdef INSTRUMENTATION
, processed_facts(0), sent_facts(0), ins_state(statistics::NOW_ACTIVE)
#endif
{

}

}
