all:
	g++ -DCOMPILE_MPIT=1 -I/usr/local/lib/ -I/usr/include/openmpi/ tutorial.cpp -o tut -lboost_mpi-mt -lboost_serialization-mt -lmpi -lmpi_cxx

test: tut
	mpirun -n 2 tut
