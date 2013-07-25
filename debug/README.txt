
<==============================================================>

DEBUGGER MANUAL

Please email for questions:
Dave Campbell --dncswim76@gmail.com
7/19/2013

<==============================================================>

7/19/2013
NOTE:  The debugger currently only works in VM debugging mode, and with -n 2 in
MPI debugging mode.  The simulator debugging mode is not working yet

********************************************************************************
A.   HOW TO RUN THE DEBUGGER
********************************************************************************

     I. VM debugging MODE:

        To run the debugger in VM debugging mode, the program must be compiled
     using g++.  To run in VM mode, enter:

        ./meld -c sl -f <meld program. -D VM

     II. MPI debugging MODE:

         To run the debugger in mpi debugging mode, the program must be compiled
     using the mpic++ compiler.  Once compiled, you may execute the program
     by entering:

        mpiexec -n <# of processes> ./meld -c sl -f <meld program> -D MPI

     This will set the debugger into MPI debugging mode with the -D flag, given
     the specified number of processes.

     NOTE:  the number of VMs run will be <# of processes> - 1 because the 0th
     process is reserved for the debugging controller that shows the debugging
     prompt.  For example, -n 3 will be running two VMs in debugging mode.

     NOTE: running with -n 2 will be the same as running in VM debugging mode.

     NOTE: Also, to be able to run the boost mpi, you will need to download the
     'openmpi-bin' package.

     III. SIMULATOR Debugging MODE

        <TO BE DETERMINED>


********************************************************************************
B.  THE DEBUGGING INTERFACE
********************************************************************************

    I. Setting Breakpoints

        A Breakpoint has six different types:
                     1. factDer (fact Derivation)
                     2. factCon (fact Consumation)
                     3. factRet (fact Retraction)
                     4. sense
                     5. block
                     6. action

         To specify that you want to run the break point, type in the command:

            >break <type>
                ex.
                >break factDer

                This will tell all nodes in the system to break when a fact
                is derived.

         You can also specify the name of the type by inserting a colon after
         the type.

            >break <type>:<name>
                ex.
                >break factCon:iterations

                This will tell all nodes in the system to break when a fact with
                the name iterations has been consumed.

         Finally, you can specify the node that you want to set the break point
         at by inserting it after the '@' character.

            >break <type>:<name>@<node#>
                ex.
                >break factRet:elem@3

                This will tell the certain node that fact name elem as been
                retracted at node 3.

            OR

            >break <type>@<node#>
                ex.
                >break sense@3

                This will tell node 3 to break if it has any kind of sense.


      II.  Dumping The System

          To dump the contents of the system:

             >dump all
             -This will dump all the nodes in the system.

             >dump <node#>
             -This will dump the contents of a certain node.

      III.  Removing breakpoints

          To remove a breakpoint, enter:

          >rm <specification>
          The specification follows that exact same way you would decribe a
          a breakpoint.
            ex.  rm factDer:iterations@3

      IV.  Printing the breakpoint list

          To print your currently input breakpoints, enter:

             >print

       V.   Quitting the debugger

          To leave to debugger,

             >quit

       VI. To continue execution of a paused VM:

             >run

             OR

             >continue


      VII.  IMPORTANT NOTES

          -When the debugger is in mpi debugging mode, the feedback will also
          tell you which VM the information came from.

          -The types sense, action, and block are really only specific to
            the simulation debugger.  If they are input in another mode,
            they will be ignored, and the program will run as if
            they are not there.

          -The block type sets a break point whenever a certain block executes
             It is really only applicable to use block@<node#> to specify which
             block you are referring to.

********************************************************************************
C.  HOW THE DEBUGGER IS IMPLEMENTED
********************************************************************************

            When a breakpoint is input, the debugger will enter it into a list
        of breakpoints. As the program executes, at specific locations, it will
        trap the program into a loop until the debugging controlling thread
        breaks it out. while paused, the debugger can asto print the system
        as well as print out other information like the fact list.

            The function runBreakPoint acts as a filter that checks to see if
        a breakpoint was reached and is interted into the VM code.

            In MPI debugging mode, the debugger uses the boost interface to send
        messages across multiple VMs.  When sending to all nodes, the debugger
        will request to the api to broadcast the signal as opposed to just one
        signal.  Each separate VM will have its own break point list.  When the
        api gets a debug message, it will push it into the debugger messageQueue.

            Also, in the MPI debugging mode, PROCESS 0 is reserved as the
        controlling process.  So the VMs will be numbered from 1 to N-1.

            In SIM debugging mode,  there is a controlling THREAD that requests
        the simulator to send messages over the api.

                         When in simulating mode, the debugger expects to
                         recieve an expected number if messages back to ensure
                         accurate synchronization of the input prompt.

        For further inquery of how the debugger is implemented see:

              debug/debug_handler.cpp
              debug/debug_prompt.cpp
              debug/debug_list.cpp

              vm/exec.cpp (where the factDer and factCon break points are set)

              vm/state.cpp (where the factRet break point is placed)

              process/machine.cpp (where action/sense breakpoints are placed)
                             **NOT IMPLEMENTED YET**

              sched/base.cpp (where the block break point will be placed)
                             **NOT IMPLEMENTED YET**
