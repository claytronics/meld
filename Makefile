include conf.mk

OS = $(shell uname -s)

INCLUDE_DIRS = -I.
LIBRARY_DIRS =

#to remove depricated char* warnings
NOSTRINGWARN = -Wno-write-strings

ARCH = -march=x86-64
#ARCH = -march=armv6
ifeq ($(RELEASE), true)
	DEBUG =
	OPTIMIZATIONS = -O3 -DNDEBUG
else
	DEBUG = -g
	PROFILING = -pg
	OPTIMIZATIONS = -O0
endif

WARNINGS = -Wall -Wextra #-Werror
C0X = -std=c++0x


ifeq ($(INTERFACE),true)
	LIBRARIES = -lwebsocketpp -ljson_spirit
	CFLAGS += -DUSE_UI=1
endif

CFLAGS = $(ARCH) $(PROFILING) $(OPTIMIZATIONS) $(WARNINGS) $(DEBUG) $(INCLUDE_DIRS) $(C0X) $(NOSTRINGWARN) -DUSERFRIENDLY=1
LIBRARIES = -pthread -lpthread -lm  -lboost_thread -lboost_system \
				-lboost_date_time -lboost_regex	\
				-ldl $(UILIBRARIES)

GCC_MINOR    := $(shell $(CXX) -v 2>&1 | grep " version " | cut -d' ' -f3  | cut -d'.' -f2)

ifeq ($(GCC_MINOR),2)
	CFLAGS += -DTEMPLATE_OPTIMIZERS=1
endif
ifeq ($(GCC_MINOR),4)
	CFLAGS += $(C0X)
endif

CXXFLAGS = $(CFLAGS)
LDFLAGS = $(PROFILING) $(LIBRARY_DIRS) $(LIBRARIES)
COMPILE = $(CXX) $(CXXFLAGS) 

SRCS = 	vm/exec.cpp \
	vm/dummy-external.cpp \
	vm/dummy-program.cpp \
	vm/dummy-state.cpp \
	vm/dummy-rule_matcher.cpp \
	vm/dummy-tuple.cpp \
	vm/dummy-predicate.cpp \
	vm/dummy-types.cpp \
	mem/dummy-center.cpp \
	process/dummy-machine.cpp \
	runtime/dummy-common.cpp \
	db/dummy-node.cpp \
	db/dummy-tuple.cpp \
	db/dummy-database.cpp \
	db/dummy-agg_configuration.cpp \
	db/dummy-trie.cpp \
	debug/dummy-debug_handler.cpp \
	dummy-meld.cpp

export

all:	meld-bbsim

meld-%:	FORCE
	@echo "Making meld-$* with Makefile.$*"
	@mkdir -p target/$*
	@$(MAKE) --no-print-directory -f Makefile.$* TARGET=$*

print:

clean:
	/bin/rm -rf target

FORCE:


