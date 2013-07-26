
#ifndef CONF_HPP
#define CONF_HPP

#include <cstdlib>

/* when this is active we active a more strict debugging of threads */
// #define ASSERT_THREADS 1

/* when this is active the allocator checks for allocate/dealloc correctness */
// #define ALLOCATOR_ASSERT 1

/* when this is active it activates extra trie checking code */
// #define TRIE_ASSERT 1

/* activate this to collect statistics on memory use */
//#define MEMORY_STATISTICS 1

/* activate debug mode */
//#define DEBUG_MODE 1

/* activate special code for testing trie matching */
// #define TRIE_MATCHING_ASSERT 1

/* gather statistics about the core VM execution */
//#define CORE_STATISTICS 1

/* use fact counting for rule engine */
#define USE_RULE_COUNTING 1

/* activate instrumentation code */
// #define INSTRUMENTATION 1

//#define DEBUG_SAFRAS 1
//#define DEBUG_REMOTE 1
//#define DEBUG_ACTIVE 1
//#define DEBUG_SERIALIZATION_TIME 1

/* build hash table of nodes for work stealing schedulers */
#define MARK_OWNED_NODES

/* use ui interface */
// #define USE_UI

/* use simulator */
#define USE_SIM

/* use memory pools for each thread or not */
const bool USE_ALLOCATOR = true;

/* rounds to delay after we fail to find an active worker */
const size_t DELAY_STEAL_CYCLE(10);
/* factor to compute the number of nodes to send to another worker when stealing */
const size_t STEAL_NODES_FACTOR(1000);

/* threshold to use in global/threads_static to flush work to other threads */
const size_t THREADS_GLOBAL_WORK_FLUSH(20);

#endif
