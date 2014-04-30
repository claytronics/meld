
#include <iostream>
#include <assert.h>

#include "db/node.hpp"
#include "vm/state.hpp"
#include "utils/utils.hpp"
#include "debug/debug_handler.hpp"

using namespace db;
using namespace std;
using namespace vm;
using namespace utils;

namespace db
{
  void
  node::match_predicate(const predicate_id id, tuple_vector& vec) const
  {
  }

  void
  node::match_predicate(const predicate_id id, const match& m, tuple_vector& vec) const
  {
  }

  void
  node::delete_all(const predicate*)
  {
  }

  void
  node::delete_by_leaf(const predicate *pred, tuple_trie_leaf *leaf, const depth_t depth)
  {
  }

  void
  node::delete_by_index(const predicate *pred, const match& m)
  {
  }

  node::~node(void)
  {
  }

  void
  node::assert_end(void) const
  {
  }
}
