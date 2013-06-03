
#include "sched/common.hpp"
#include "db/database.hpp"
#include "vm/state.hpp"

using namespace vm;
using namespace db;
using namespace process;

namespace sched
{

#ifndef NDEBUG
void
assert_static_nodes_end_iteration(const process_id id, vm::all *all)
{
   const node::node_id first(0);
   const node::node_id final(all->DATABASE->nodes_total - 1);
   database::map_nodes::const_iterator it(all->DATABASE->nodes_begin());
   database::map_nodes::const_iterator end(all->DATABASE->nodes_end());

   for(; it != end; ++it)
      it->second->assert_end_iteration();
}

void
assert_static_nodes_end(const process_id id, vm::all *all)
{
   const node::node_id first(0);
   const node::node_id final(all->DATABASE->nodes_total - 1);
   database::map_nodes::const_iterator it(all->DATABASE->nodes_begin());
   database::map_nodes::const_iterator end(all->DATABASE->nodes_end());

   for(; it != end; ++it)
      it->second->assert_end();
}
#endif

}
