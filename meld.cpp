#include <iostream>
#include <vector>

#include "process/machine.hpp"
#include "utils/utils.hpp"
#include "utils/fs.hpp"
#include "vm/state.hpp"
#include "debug_handler.hpp"
#include "debug_prompt.hpp"
#include "interface.hpp"

using namespace utils;
using namespace process;
using namespace std;
using namespace sched;

static char *program = NULL;
static char *progname = NULL;

static void
help(void)
{
	cerr << "meld: execute meld program" << endl;
	cerr << "meld -f <program file> -c <scheduler> -- arg1 arg2 ... argN" << endl;
	cerr << "\t-f <name>\tmeld program" << endl;
	help_schedulers();
	cerr << "\t-t \t\ttime execution" << endl;
	cerr << "\t-m \t\tmemory statistics" << endl;
	cerr << "\t-i <file>\tdump time statistics" << endl;
	cerr << "\t-s \t\tshows database" << endl;
   cerr << "\t-d \t\tdump database (debug option)" << endl;
   cerr << "\t-h \t\tshow this screen" << endl;
   cerr << "\t-D \t\tgo into debugging mode" << endl;
   cerr << "\t-S \t\tused to go into debugging mode called from simulator" << endl;

   exit(EXIT_SUCCESS);
}

static vm::machine_arguments
read_arguments(int argc, char **argv)
{
	vm::machine_arguments program_arguments;
	
   progname = *argv++;
   --argc;

   while (argc > 0 && (argv[0][0] == '-')) {
      switch(argv[0][1]) {
         case 'f': {
            if (program != NULL || argc < 2)
               help();

            program = argv[1];

            argc--;
            argv++;
         }
         break;
         case 'c': {
            if (sched_type != SCHED_UNKNOWN)
               help();
            
            parse_sched(argv[1]);
            argc--;
            argv++;
         }
         break;
         case 's':
            show_database = true;
            break;
         case 'd':
            dump_database = true;
            break;
         case 't':
            time_execution = true;
            break;
         case 'm':
            memory_statistics = true;
            break;
         case 'i':
            if(argc < 2)
               help();
               
            statistics::set_stat_file(string(argv[1]));
            argc--;
            argv++;
            break;
         case 'h':
            help();
            break;
         case 'D':
	   cout << "DEBUGGING MODE- type help for options" << endl;
	   setDebuggingMode(true);
	   break;
         case 'S':
	   setSimDebuggingMode(true);
	   break;
	
	   
			case '-':
				
				for(--argc, ++argv ; argc > 0; --argc, ++argv)
					program_arguments.push_back(string(*argv));
			break;
         default:
            help();
      }

      /* advance */
      argc--; argv++;
   }

	return program_arguments;
}

int
main(int argc, char **argv)
{
   vm::machine_arguments margs(read_arguments(argc, argv));

   if(sched_type == SCHED_UNKNOWN) {
      sched_type = SCHED_SERIAL;
      num_threads = 1;
   }

   if(program == NULL && sched_type != SCHED_UNKNOWN) {
		cerr << "Error: please provide a program to run" << endl;
      return EXIT_FAILURE;
   }
   
	if(!file_exists(program)) {
		cerr << "Error: file " << program << " does not exist or is not readable" << endl;
		return EXIT_FAILURE;
	}
	
   try {
      run_program(argc, argv, program, margs);
   } catch(vm::load_file_error& err) {
      cerr << "File error: " << err.what() << endl;
      exit(EXIT_FAILURE);
   } catch(machine_error& err) {
      cerr << "VM error: " << err.what() << endl;
      exit(EXIT_FAILURE);
   } catch(db::database_error& err) {
      cerr << "Database error: " << err.what() << endl;
      exit(EXIT_FAILURE);
   }

   return EXIT_SUCCESS;
}
