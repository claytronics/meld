
#include <iostream>

#include <cstdlib>
#include <cstring>
#include <assert.h>

#include "interface.hpp"
#include "stat/stat.hpp"
#include "utils/time.hpp"
#include "utils/fs.hpp"
#include "process/machine.hpp"
#include "api/api.hpp"

using namespace process;
using namespace sched;
using namespace std;
using namespace utils;
using namespace vm;

scheduler_type sched_type = SCHED_UNKNOWN;
size_t num_threads = 0;
bool show_database = false;
bool dump_database = false;
bool time_execution = false;
bool memory_statistics = false;

static inline size_t
num_cpus_available(void)
{
    return (size_t)sysconf(_SC_NPROCESSORS_ONLN );
}

static inline bool
match_serial(const char *name, char *arg, const scheduler_type type)
{
   const size_t len(strlen(name));
   
   if(strlen(arg) == len && strncmp(name, arg, len) == 0) {
      sched_type = type;
      num_threads = 1;
      return true;
   }
   
   return false;
}

static inline bool
fail_sched(char* sched)
{
	cerr << "Error: invalid scheduler " << sched << endl;
   exit(EXIT_FAILURE);
   return false;
}

void
parse_sched(char *sched)
{
   assert(sched != NULL);
   
   if(strlen(sched) < 2)
      fail_sched(sched);
   
   // attempt to parse the scheduler string
      match_serial("sl", sched, SCHED_SERIAL) ||
		match_serial("ui", sched, SCHED_SERIAL_UI) ||
      fail_sched(sched);

	if (num_threads == 0) {
      cerr << "Error: invalid number of threads" << endl;
      exit(EXIT_FAILURE);
   }
}

void
help_schedulers(void)
{
	cerr << "\t-c <scheduler>\tselect scheduling type" << endl;
	cerr << "\t\t\tsl simple serial scheduler" << endl;
	cerr << "\t\t\tui serial scheduler + ui" << endl;
   cerr << "\t\t\tthX multithreaded scheduler with task stealing" << endl;
   cerr << "\t\t\tthpX multithreaded scheduler with priorities and task stealing" << endl;
}

static inline void
finish(void)
{
}

/* program = meld program */
bool
run_program(int argc, char **argv, const char *program, const vm::machine_arguments& margs)
{
	assert(utils::file_exists(string(program)));
	assert(num_threads > 0);

	try {
        /* save time to compute execution time */
        /* calculate execution time */
      double start_time(0.0);
      execution_time tm;

      if(time_execution) {
            tm.start();
      }

        boost::mpi::environment env(argc, argv);
        boost::mpi::communicator world;
        api::world = &world;

      /* instantiate machine
       * serial: 1 thread, sched_serial
       * margs: meld argv, argc*,
       * each machine will have a mpi communicator in machine->WORLD
       */
      machine mac(program, num_threads, sched_type, margs);

      /* Creat barrier here, to prevent processes from sneding messages to
       * processes that have not been started yet */
      api::world->barrier();

      /* initiates threads */
      mac.start();

      if(time_execution) {
         {
            tm.stop();
            size_t ms = tm.milliseconds();
            cout << "Time: " << ms << " ms" << endl;
         }
      }

	} catch(machine_error& err) {
      finish();
      throw err;
   } catch(load_file_error& err) {
      finish();
      throw err;
   } catch(db::database_error& err) {
      finish();
      throw err;
   }

   finish();

	return true;
}
