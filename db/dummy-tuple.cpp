#include <assert.h>

#include "db/tuple.hpp"

using namespace db;
using namespace vm;
using namespace std;
using namespace boost;
using namespace utils;

namespace db
{

void
simple_tuple::pack(byte *buf, const size_t buf_size, int *pos) const
{
}

simple_tuple*
simple_tuple::unpack(byte *buf, const size_t buf_size, int *pos, vm::program *prog)
{
}

simple_tuple::~simple_tuple(void)
{
   // simple_tuple is not allowed to delete tuples
}

void
simple_tuple::print(ostream& cout) const
{
}

ostream& operator<<(ostream& cout, const simple_tuple& tuple)
{
}

}
