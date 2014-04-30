#include <cstdlib>
#include <assert.h>

#include "vm/types.hpp"
#include "vm/defs.hpp"
#include "utils/utils.hpp"

using namespace vm;
using namespace std;
using namespace utils;

namespace vm {

  type *TYPE_INT(new type(FIELD_INT));
  type *TYPE_FLOAT(new type(FIELD_FLOAT));
  type *TYPE_NODE(new type(FIELD_NODE));
  type *TYPE_STRING(new type(FIELD_STRING));
  list_type *TYPE_LIST_INT(new list_type(new type(FIELD_INT)));
  list_type *TYPE_LIST_FLOAT(new list_type(new type(FIELD_FLOAT)));
  list_type *TYPE_LIST_NODE(new list_type(new type(FIELD_NODE)));
   
  string
  field_type_string(field_type type)
  {
  }
}
