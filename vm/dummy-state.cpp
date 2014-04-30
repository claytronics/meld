
#include <iostream>
#include "vm/state.hpp"
#include "process/machine.hpp"
#include "vm/exec.hpp"
#include "debug/debug_handler.hpp"
#include "api/api.hpp"

using namespace vm;
using namespace db;
using namespace process;
using namespace std;
using namespace runtime;
using namespace utils;

namespace vm
{

bool
state::linear_tuple_can_be_used(db::tuple_trie_leaf *leaf) const
{
}

void
state::using_new_linear_tuple(db::tuple_trie_leaf *leaf)
{
}

void
state::no_longer_using_linear_tuple(db::tuple_trie_leaf *leaf)
{
}

void
state::unmark_generated_tuples(void)
{
}

void
state::cleanup(void)
{
}

void
state::copy_reg2const(const reg_num& reg_from, const const_id& cid)
{
}

void
state::mark_predicate_to_run(const predicate *pred)
{
}

}
