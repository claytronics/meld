
#ifndef SCHED_TYPES_HPP
#define SCHED_TYPES_HPP

namespace sched
{
   
enum scheduler_type {
   SCHED_UNKNOWN,
   SCHED_THREADS,
   SCHED_THREADS_PRIO,
   SCHED_SERIAL,
	SCHED_SERIAL_UI,
	SCHED_SIM
};

inline bool is_serial_sched(const scheduler_type type)
{
   return type == SCHED_SERIAL || type == SCHED_SERIAL_UI || type == SCHED_SIM;
}


// keeping only sched_serial.

inline bool is_mpi_sched(const scheduler_type)
{
   return false;
} 

} 
  


#endif
