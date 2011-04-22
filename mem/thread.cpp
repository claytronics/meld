
#include <boost/thread/tss.hpp>
#include <vector>
#include <iostream>

#include "mem/thread.hpp"

using namespace boost;
using namespace std;

namespace mem
{
   
static thread_specific_ptr<pool> pools(NULL);
static vector<pool*> vec;

void
init(const size_t num_threads)
{
   vec.resize(num_threads);
}

void
create_pool(const size_t id)
{
   pool *pl(new pool());   
   pools.reset(pl);
   vec[id] = pl;
}

pool*
get_pool(void)
{
   return pools.get();
}

void
cleanup(const size_t num_threads)
{
   for(size_t i(0); i < num_threads; ++i)
      delete vec[i];
}

}