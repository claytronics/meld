
#include <iostream>
#include <boost/thread/mutex.hpp>

#include "process/router.hpp"
#include "vm/state.hpp"
#include "db/database.hpp"
#include "sched/mpi/handler.hpp"

using namespace process;
using namespace boost;
using namespace vm;
using namespace std;
using namespace db;
using namespace utils;
using namespace sched;

namespace process
{
   
static mutex mpi_mutex;
   
void
router::set_nodes_total(const size_t total, vm::all *all)
{
   nodes_per_remote = total / world_size;
   
#ifndef USE_SIM
   if(nodes_per_remote == 0)
      throw database_error("Number of nodes is less than the number of remote machines");
#endif

   all->NUM_NODES_PER_PROCESS = nodes_per_remote;

   // cache values for each remote
   for(remote::remote_id i(0); i != (remote::remote_id)world_size; ++i)
      remote_list[i]->cache_values(world_size, nodes_per_remote, total);
}

remote*
router::find_remote(const node::node_id id) const
{
   (void)id;
   return remote::self;
}

void
router::base_constructor(const size_t num_threads, int argc, char **argv, const bool use_mpi)
{
   (void)argc;
   (void)argv;
   (void)use_mpi;
   {
      world_size = 1;
      remote_list.resize(world_size);
      remote_list[0] = new remote(0, num_threads);
      remote::self = remote_list[0];
   }
   
   remote::world_size = world_size;
   
}

router::router(void)
{
   base_constructor(1, 0, NULL, false);
}

router::router(const size_t num_threads, int argc, char **argv, const bool use_mpi)
{
   base_constructor(num_threads, argc, argv, use_mpi);
}

router::~router(void)
{
   for(remote::remote_id i(0); i != (remote::remote_id)world_size; ++i)
      delete remote_list[i];
}

}
