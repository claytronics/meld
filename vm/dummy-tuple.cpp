
#include <assert.h>
#include <iostream>

#include "vm/tuple.hpp"
#include "db/node.hpp"
#include "utils/utils.hpp"
#include "vm/state.hpp"
#include "utils/serialization.hpp"
#include "debug/debug_handler.hpp"

using namespace vm;
using namespace std;
using namespace runtime;
using namespace utils;
using namespace boost;

namespace vm
{
  tuple::tuple(const predicate* _pred):
    pred((predicate*)_pred), fields(allocator<tuple_field>().allocate(pred->num_fields()))
  {
  }

  size_t
  tuple::get_storage_size(void) const
  {
  }

  tuple::~tuple(void)
  {
  }

  ostream& operator<<(ostream& cout, const vm::tuple& tuple)
  {
  }
}
