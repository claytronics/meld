#include <algorithm>
#include <iostream>

#include "vm/rule_matcher.hpp"
#include "vm/program.hpp"
#include "vm/state.hpp"
#include "api/api.hpp"

using namespace std;
using namespace db;

namespace vm
{
  /* returns true if we did not have any tuples of this predicate */
  bool
  rule_matcher::register_tuple(tuple *tpl, const derivation_count count, const bool is_new)
  {
  }

  /* returns true if now we do not have any tuples of this predicate */
  bool
  rule_matcher::deregister_tuple(tuple *tpl, const derivation_count count)
  {
  }
}
