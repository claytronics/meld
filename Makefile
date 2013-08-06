OS = $(shell uname -s)

INCLUDE_DIRS = -I.
LIBRARY_DIRS =

ifeq (exists, $(shell test -d /opt/local/include && echo exists))
	INCLUDE_DIRS += -I/opt/local/include
endif
ifeq (exists, $(shell test -d /opt/local/lib  && echo exists))
	LIBRARY_DIRS += -L/opt/local/lib
endif

PROFILING = #-pg
OPTIMIZATIONS = -O0
ARCH = -march=x86-64
#DEBUG = -g -DDEBUG_RULES
WARNINGS = -Wall -Wextra #-Werror
C0X = -std=c++0x
UILIBRARIES = #-lwebsocketpp -ljson_spirit

CFLAGS = $(ARCH) $(PROFILING) $(OPTIMIZATIONS) $(WARNINGS) $(DEBUG) $(INCLUDE_DIRS) $(COX)

LIBRARIES = -pthread -lpthread -lm -lreadline -lboost_thread-mt -lboost_system-mt \
			-lboost_date_time-mt -lboost_regex-mt $(UILIBRARIES)

LIBRARIES +=  -lboost_serialization-mt -lboost_mpi-mt

MPICPP = $(shell mpic++ --version > /dev/null && echo exists)

ifeq (exists, $(MPICPP))
	CXX = mpic++
else
	CXX = g++
endif

GCC_MINOR    := $(shell $(CXX) -v 2>&1 | grep " version " | cut -d' ' -f3  | cut -d'.' -f2)

ifeq ($(GCC_MINOR),2)
	CFLAGS += -DTEMPLATE_OPTIMIZERS=1
endif
ifeq ($(GCC_MINOR),4)
	CFLAGS += $(C0X)
endif

CXXFLAGS = $(CFLAGS)
LDFLAGS = $(PROFILING) $(LIBRARY_DIRS) $(LIBRARIES)
COMPILE = $(CXX) $(CXXFLAGS) $(OBJS)

SRCS = utils/utils.cpp \
		 	utils/types.cpp \
			utils/fs.cpp \
			 vm/program.cpp \
			 vm/predicate.cpp \
			 vm/types.cpp \
			 vm/instr.cpp \
			 vm/state.cpp \
			 vm/tuple.cpp \
			 vm/exec.cpp \
			 vm/external.cpp \
			 vm/rule.cpp \
			 vm/rule_matcher.cpp \
			 db/node.cpp \
			 db/tuple.cpp \
			 db/agg_configuration.cpp \
			 db/tuple_aggregate.cpp \
			 db/database.cpp \
			 db/trie.cpp \
			 process/machine.cpp \
			 mem/thread.cpp \
			 mem/center.cpp \
			 mem/stat.cpp \
			 sched/base.cpp \
			 sched/common.cpp \
			 sched/serial.cpp \
			 sched/thread/threaded.cpp \
			 sched/thread/assert.cpp \
			 external/math.cpp \
			 external/lists.cpp \
			 external/utils.cpp \
			 external/strings.cpp \
			 external/others.cpp \
			 external/core.cpp \
			 stat/stat.cpp \
			 stat/slice.cpp \
			 stat/slice_set.cpp \
			 interface.cpp \
			 debug/debug_prompt.cpp \
			 debug/debug_handler.cpp \
			 debug/debug_list.cpp \
			 api/bbsimapi.cpp
#			 api/bbsimapi.cpp \
#			 api/mpi.cpp \


OBJS = $(patsubst %.cpp,%.o,$(SRCS))

all: meld print server

	echo $(OBJS)

-include Makefile.externs
Makefile.externs:	Makefile
	@echo "Remaking Makefile.externs"
	@/bin/rm -f Makefile.externs
	@for i in $(SRCS); do $(CXX) $(CXXFLAGS) -MM -MT $${i/%.cpp/.o} $$i >> Makefile.externs; done
	@echo "Makefile.externs ready"

meld: $(OBJS) meld.o
	$(COMPILE) meld.o -o meld $(LDFLAGS)

print: $(OBJS) print.o
	$(COMPILE) print.o -o print $(LDFLAGS)

server: $(OBJS) server.o
	$(COMPILE) server.o -o server $(LDFLAGS)

depend:
	makedepend -- $(CXXFLAGS) -- $(shell find . -name '*.cpp')

clean:
	find . -name '*.o' | xargs rm -f
	rm -f meld predicates print server Makefile.externs
# DO NOT DELETE
