// call addCompileInfo with string describing the options being compiled into the program.
#ifndef _COMPILE_INFO_
#define COMPILE_INFO

namespace utils {

#ifndef BBHW
char* addCompileInfo(char const* info);
#define declareCompileInfo(x, y) char* x = utils::addCompileInfo(y)
#else
#define addCompileInfo(x)
#define declareCompileInfo(x, y) 
#endif

}

#endif


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
