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
#include "debug/debug_handler.hpp"
#include "debug/debug_prompt.hpp"


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
  }

  void
  machine::route_self(sched::base *sched, node *node, simple_tuple *stpl, const uint_val delay)
  {
  }

  void
  machine::route(const node* from, sched::base *sched_caller, const node::node_id id, simple_tuple* stpl, const uint_val delay)
  {
  }
}
