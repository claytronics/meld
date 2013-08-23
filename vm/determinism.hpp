#ifndef VM_DETERMINISM_HPP
#define VM_DETERMINISM_HPP

#include <cstdio>
#include "db/node.hpp"
#include "vm/instr.hpp"

namespace vm {
	
	namespace determinism {
		typedef size_t deterministic_timestamp;
		enum simulationMode {DETERMINISTIC1 = 1, DETERMINISTIC2 = 2, REALTIME = 3};
		
		void setSimulationMode(simulationMode mode);
		simulationMode getSimulationMode();
		//bool isInDeterministicMode();
		bool canCompute();
		bool isComputing();
		void checkAndWaitUntilCanCompute();
		bool mustQueueMessages();

		void resumeComputation(deterministic_timestamp ts, deterministic_timestamp d);
		void computationPause();
		void workEnd();

		deterministic_timestamp getCurrentLocalTime();
		void incrCurrentLocalTime(pcounter pc);
		void setCurrentLocalTime(deterministic_timestamp time);
	}
}
#endif
