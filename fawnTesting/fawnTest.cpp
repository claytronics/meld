
/*BOOST MPI INTERFACE TESTING*/

/* DAVID CAMPBELL - dncswim76@gmail.com*/

/*This test consist of computing data repeatedly and
  then sending a message.  When rendezvous is specified,
  it will wait for the message to come.  If it is not
  specified it send a message and then collect them at the
  end.  */




#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
#include <iostream>
#include <string>
#include <boost/serialization/string.hpp>
#include <stdlib.h>

namespace mpi = boost::mpi;

#define SENDNEXT 0
#define WAIT 1

using namespace std;


int nextProcess(mpi::communicator world){
    return (world.rank()+1)%world.size();
}

int prevProcess(mpi::communicator world){
    if (world.rank() == 0){
        return world.size()-1;
    } else {
        return world.rank()-1;
    }
}



int main(int argc, char* argv[])
{

    /*outer loop repatitions*/
    int L = atoi(argv[1]);

    /*inner computation loop*/
    int l = atoi(argv[2]);

    /*if rendezvous*/
    int rdvs = atoi(argv[3]);

    /*init boost mpi*/
    mpi::environment env(argc, argv);
    mpi::communicator world;


    /*outer loop - number of messages to be sent*/
    int sum = 0;
    for (int i = 0; i < L; i++){
        /*inner loop - data computation*/
        for (int j = 0; j < l; j++){
            sum += j;
        }
        sum = 0;

        /*send message*/
        world.send(nextProcess(world), SENDNEXT);

        if (rdvs){
            /*if rendezvous, wait for a message*/
        	world.recv(prevProcess(world), SENDNEXT);
        }
    }

    if (!rdvs){

        /*if not rendezvous, collect messages at the end*/
        while(world.iprobe(prevProcess(world),SENDNEXT)){
            world.recv(prevProcess(world),SENDNEXT);
        }
    }
    return 0;
}
