# make -f test.make FILE=<srcname> [TARGET=<mpi|bbsim|serial|mcore>] [NODES=<number>] [update]
#
# FILE = the meld program to test
# TARGET = the api used to do the test
# NODES = if relevant, the number of nodes for the target
# update = to update the reference output
#
# FILE=<srcname> must be specified, all others are optional.  if not specified, then TARGET=mpi NODES=1
# will make sure <srcname> is compiled and run the test and compare to the reference output
#
# <srcname> = a meld file in the progs directory
# 

# figure out the source file & compiled file
base=$(FILE)
basename=$(basename $(base))
mfile=code/$(basename)

# default target to test.  user can specify alternatives
TARGET=mpi

# if relevant, number of nodes to test on
NODES=1

top:	exec-$(TARGET)

exec-mpi:	$(mfile).m files/$(basename).test
	mpiexec -n $(NODES) ../meld -d -f $(mfile).m -c sl > /tmp/mpi.output
	diff /tmp/mpi.output files/$(basename).test

$(mfile).m:	progs/$(base)
	mcl.sh progs/$(base) $(mfile)
