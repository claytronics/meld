#include <iostream>
#include <signal.h>

#include "process/machine.hpp"
#include "vm/program.hpp"
#include "vm/state.hpp"
#include "vm/exec.hpp"
#include "runtime/list.hpp"
#include "mem/thread.hpp"
#include "mem/stat.hpp"
#include "stat/stat.hpp"
#include "utils/fs.hpp"
#include "interface.hpp"
#include "sched/serial.hpp"
#include "api/api.hpp"

using namespace process;
using namespace db;
using namespace std;
using namespace vm;
using namespace boost;
using namespace sched;
using namespace mem;
using namespace utils;
using namespace statistics;

namespace process
{

void
machine::run_action(sched::base *sched, node* node, vm::tuple *tpl, const bool from_other)
{
	const predicate_id pid(tpl->get_predicate_id());
	
	assert(tpl->is_action());
	
   switch(pid) {
      case SETCOLOR_PREDICATE_ID:
      case SETCOLOR2_PREDICATE_ID:
      case SETEDGELABEL_PREDICATE_ID:
      break;
      case SET_PRIORITY_PREDICATE_ID:
      if(from_other)
         sched->set_node_priority_other(node, tpl->get_float(0));
      else {
         sched->set_node_priority(node, tpl->get_float(0));
      }
      break;
      case ADD_PRIORITY_PREDICATE_ID:
      if(from_other)
         sched->add_node_priority_other(node, tpl->get_float(0));
      else
         sched->add_node_priority(node, tpl->get_float(0));
      break;
      case WRITE_STRING_PREDICATE_ID: {
         runtime::rstring::ptr s(tpl->get_string(0));

         cout << s->get_content() << endl;
        }
      break;
      case SCHEDULE_NEXT_PREDICATE_ID:
      if(!from_other) {
         sched->schedule_next(node);
      } else {
         assert(false);
      }
      break;
      default:
		assert(false);
      break;
   }

	delete tpl;
}

void
machine::route_self(sched::base *sched, node *node, simple_tuple *stpl, const uint_val delay)
{
   if(delay > 0) {
      work new_work(node, stpl);
      sched->new_work_delay(sched, node, new_work, delay);
   } else {
      assert(!stpl->get_tuple()->is_action());
      sched->new_work_self(node, stpl);
   }
}

void
machine::route(const node* from, sched::base *sched_caller, const node::node_id id, simple_tuple* stpl, const uint_val delay)
{
   assert(sched_caller != NULL);
   assert(id <= this->all->DATABASE->max_id());

   if (api::on_current_process(id)){
       /* Belongs to the same process, does not require MPI */
      node *node(this->all->DATABASE->find_node(id));

      sched::base *sched_other(sched_caller->find_scheduler(node));
        const predicate *pred(stpl->get_predicate());

      if(delay > 0) {
            work new_work(node, stpl);
         sched_caller->new_work_delay(sched_caller, from, new_work, delay);
      } else if(pred->is_action_pred()) {
            run_action(sched_other, node, stpl->get_tuple(), sched_caller != sched_other);
            delete stpl;
      } else if(sched_other == sched_caller) {
            work new_work(node, stpl);

         sched_caller->new_work(from, new_work);
      } else {
         work new_work(node, stpl);

         sched_caller->new_work_other(sched_other, new_work);
      }
   } else {
        /* Send to the correct process */
       /* isend (destination process id, tag) */
       /* TODO handle serializing the data to send over MPI */
       /* TODO Abstract MPI to API layer */
       //uint64_t reply[1000];
       //int i = 0;
        //stpl->pack((utils::byte *)reply, 1000, &i);
        //int random = rand();
       //cout << "Process " << this->all->WORLD.rank() << " :: tag " << id
           //<< " :: " << random << " ==> " << (id % this->all->WORLD.size()) << endl;
        //this->all->WORLD.isend(id % this->all->WORLD.size(), id, random);
    int dest = id % api::world->size();
    int r = rand();
    cout << "Process " << api::world->rank() << " ==> Process " << dest
	 << " :: " << *stpl << " :: " << r << endl;

     api::message_type *msg = new api::message_type[512];
     size_t msg_length = 512 * sizeof(api::message_type);

     int p = 0;

     stpl->pack((utils::byte *) msg, msg_length, &p);
     
     api::send_message(id, api::create_message(stpl), msg_length, this->all, r);
   }
}

void
machine::deactivate_signals(void)
{
   sigset_t set;
   
   sigemptyset(&set);
   sigaddset(&set, SIGALRM);
   sigaddset(&set, SIGUSR1);
   
   sigprocmask(SIG_BLOCK, &set, NULL);
}

void
machine::set_timer(void)
{
   // pre-compute the number of usecs from msecs
   static long usec = SLICE_PERIOD * 1000;
   struct itimerval t;
   
   t.it_interval.tv_sec = 0;
   t.it_interval.tv_usec = 0;
   t.it_value.tv_sec = 0;
   t.it_value.tv_usec = usec;
   
   setitimer(ITIMER_REAL, &t, 0);
}

void
machine::slice_function(void)
{
   bool tofinish(false);
   
   // add SIGALRM and SIGUSR1 to sigset
	// to be used by sigwait
   sigset_t set;
   sigemptyset(&set);
   sigaddset(&set, SIGALRM);
   sigaddset(&set, SIGUSR1);

   int sig;
   
   set_timer();
   
   while (true) {
      
      const int ret(sigwait(&set, &sig));
		
		assert(ret == 0);
      
      switch(sig) {
         case SIGALRM:
         if(tofinish)
            return;
         slices.beat(all);
         set_timer();
         break;
         case SIGUSR1:
         tofinish = true;
         break;
         default: assert(false);
      }
   }
}

void
machine::execute_const_code(void)
{
	state st(all);
	
	// no node or tuple whatsoever
	st.setup(NULL, NULL, 0);
	
	execute_bytecode(all->PROGRAM->get_const_bytecode(), st);
}

void
machine::init_thread(sched::base *sched)
{
	all->ALL_THREADS.push_back(sched);
	all->NUM_THREADS++;
	sched->start();
}

// Start all schedulers in the VM
void
machine::start(void)
{
	// execute constants code
	execute_const_code();
	
   deactivate_signals();
   
   // Statistics sampling
   if(stat_enabled()) {
      // initiate alarm thread
      alarm_thread = new boost::thread(bind(&machine::slice_function, this));
   }
   
   //for(size_t i(1); i < all->NUM_THREADS; ++i)
      //this->all->ALL_THREADS[i]->start();
   this->all->ALL_THREADS[0]->start();
   
   // Wait for threads to finish, if thread > 1
   //for(size_t i(1); i < all->NUM_THREADS; ++i)
      //this->all->ALL_THREADS[i]->join();
      
//#ifndef NDEBUG
   //for(size_t i(1); i < all->NUM_THREADS; ++i)
      //assert(this->all->ALL_THREADS[i-1]->num_iterations() == this->all->ALL_THREADS[i]->num_iterations());
   //if(this->all->PROGRAM->is_safe())
      //assert(this->all->ALL_THREADS[0]->num_iterations() == 1);
//#endif
   
   if(alarm_thread) {
      kill(getpid(), SIGUSR1);
      alarm_thread->join();
      delete alarm_thread;
      alarm_thread = NULL;
      slices.write(get_stat_file(), sched_type, all);
   }

   const bool will_print(show_database || dump_database);

   if(will_print) {
         if(show_database)
            all->DATABASE->print_db(cout);
         if(dump_database)
            all->DATABASE->dump_db(cout);
   }

   if(memory_statistics) {
#ifdef MEMORY_STATISTICS
      cout << "Total memory in use: " << get_memory_in_use() / 1024 << "KB" << endl;
      cout << "Malloc()'s called: " << get_num_mallocs() << endl;
#else
      cout << "Memory statistics support was not compiled in" << endl;
#endif
   }
}

/* Implementation specific function */
static inline database::create_node_fn
get_creation_function(const scheduler_type sched_type)
{
   switch(sched_type) {
      case SCHED_SERIAL:
         return database::create_node_fn(sched::serial_local::create_node);
      case SCHED_UNKNOWN:
         return NULL;
      default:
         return NULL;
   }
   
   throw machine_error("unknown scheduler type");
}

machine::machine(const string& file, const size_t th,
		const scheduler_type _sched_type,  const machine_arguments& margs):
   all(new vm::all()),
   filename(file),
   sched_type(_sched_type),
   alarm_thread(NULL),
   slices(th) /* th = number of threads, slices is for statistics */
{
    this->all->PROGRAM = new vm::program(file); /* predicates information, byte code for all the rules */

    if(margs.size() < this->all->PROGRAM->num_args_needed())
        throw machine_error(string("this program requires ") + utils::to_string(all->PROGRAM->num_args_needed()) + " arguments");

   this->all->MACHINE = this;
   this->all->ARGUMENTS = margs;
   this->all->DATABASE =  new database(filename, get_creation_function(_sched_type), this->all);
   this->all->NUM_THREADS = th;

   // Instantiate the scheduler object
   switch(sched_type) {
      case SCHED_SERIAL:
         this->all->ALL_THREADS.push_back(dynamic_cast<sched::base*>(new sched::serial_local(this->all)));
         break;
      case SCHED_UNKNOWN: assert(false); break;
      default: break;
   }

   assert(this->all->ALL_THREADS.size() == all->NUM_THREADS);
}

machine::~machine(void)
{
   // when deleting database, we need to access the program,
   // so we must delete this in correct order
   delete this->all->DATABASE;
   
   for(process_id i(0); i != all->NUM_THREADS; ++i)
      delete all->ALL_THREADS[i];

   delete this->all->PROGRAM;
      
   if(alarm_thread)
      delete alarm_thread;
      
   mem::cleanup(all->NUM_THREADS);
}

}
