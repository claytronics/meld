
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include "process/shm.hpp"

using namespace vm;
using namespace utils;

namespace process
{
   
#define DEFAULT_MESSAGE_TYPE 1
   
void
shm::send(process_id to, byte* buf, const size_t size)
{
   long int *ptr((long int*)buf);
   
   *ptr = DEFAULT_MESSAGE_TYPE; // the type does not matter since each process has its own queue
   
   int ret = msgsnd(others[to], buf, size + sizeof(long int), 0);
   
   assert(ret == 0);
}

bool
shm::try_receive(byte *buf, size_t *read)
{
   ssize_t ret(msgrcv(msqid, private_buf, MAX_MSG_SIZE, DEFAULT_MESSAGE_TYPE, IPC_NOWAIT));
   
   if(ret == (ssize_t)-1)
      return false;
   
   *read = (size_t)ret;
   memcpy(buf, private_buf + sizeof(long int), ret);
   
   return true;
}

void
shm::setup_queues(void)
{
   for(size_t i(0); i < num_procs; ++i) {
      if(i != (size_t)id) {
         key_t key(ftok("/tmp", (int)i));
         assert(key != -1);
         others[i] = msgget(key, 0644);
         assert(others[i] != -1);
         printf("%d: Got key of proc %d: %d\n", (int)id, (int)i, others[i]);
      }
   }
}

shm::shm(const process_id _id, const size_t world_size):
   id(_id), num_procs(world_size)
{
   key_t key = ftok("/tmp", (int)id);
   
   assert(key != -1);
   
   msqid = msgget(key, 0644 | IPC_CREAT);
   
   assert(msqid != -1);
   
   printf("Msq ID for %d: %d\n", (int)id, (int)msqid);
   
   others.resize(world_size);
}

shm::~shm(void)
{
   msgctl(msqid, IPC_RMID, NULL);
}

}