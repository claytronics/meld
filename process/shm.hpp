
#ifndef PROCESS_SHM_HPP
#define PROCESS_SHM_HPP

#include <sys/msg.h>
#include <vector>

#include "conf.hpp"
#include "vm/defs.hpp"
#include "utils/types.hpp"

namespace process
{
   
class shm
{
private:
   
   const vm::process_id id;
   const size_t num_procs;
   
   int msqid;
   std::vector<int> others;
   
   utils::byte private_buf[sizeof(long int) + MAX_MSG_SIZE];
   
public:
   
   bool send(vm::process_id, utils::byte *, const size_t);
   
   bool try_receive(utils::byte*, size_t*);
   
   void setup_queues(void);
   
   explicit shm(const vm::process_id,
      const size_t);
   
   virtual ~shm(void);
};

}

#endif
