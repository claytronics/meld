#include <assert.h>
#include <iostream>

#include "vm/predicate.hpp"
#include "vm/state.hpp"

using namespace std;
using namespace vm;
using namespace utils;

namespace vm {

  type*
  read_type_from_reader(code_reader& read)
  {
  }

  type*
  read_type_id_from_reader(code_reader& read, const vector<type*>& types)
  {
  }

  predicate*
  predicate::make_predicate_from_reader(code_reader& read, code_size_t *code_size, const predicate_id id,
					const uint32_t major_version, const uint32_t minor_version,
					const vector<type*>& types)
  {
  }

  void
  predicate::cache_info(vm::program *prog)
  {
  }

  predicate::predicate(void)
  {
    tuple_size = 0;
    agg_info = NULL;
  }

  predicate::~predicate(void)
  {
  }
}
