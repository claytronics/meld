#include <iostream>
#include <vector>

#include "process/machine.hpp"
#include "utils/utils.hpp"
#include "utils/fs.hpp"
#include "vm/state.hpp"
#include "debug/debug_handler.hpp"
#include "debug/debug_prompt.hpp"
#include "interface.hpp"
#include "compileInfo.hpp"

using namespace utils;
using namespace process;
using namespace std;
using namespace sched;

static char *program = NULL;
static char *data_file = NULL;
static char *progname = NULL;
static char checkedApiTarget = 0; // if set to 1, can exit successfully with zero args

namespace xutils {
  char*
  addCompileInfo(char const* info)
  {
  }
}

static void
help(void)
{
}

static vm::machine_arguments
read_arguments(int argc, char **argv)
{
}

int
main(int argc, char **argv)
{
  return EXIT_SUCCESS;
}


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
