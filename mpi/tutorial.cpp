#include <boost/mpi.hpp>
#include <iostream>
#include <string>
#include <boost/serialization/string.hpp>
#include <unistd.h>

namespace mpi = boost::mpi;

int main(int argc, char *argv[]) {
    mpi::environment env(argc, argv);
    mpi::communicator world;

    if (world.rank() == 0)
        sleep(10);

    world.barrier();
    std::cout << "Here: " << world.rank() << std::endl;
    return 0;
}
