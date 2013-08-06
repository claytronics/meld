#ifndef VM_DETERMINISM_HPP
#define VM_DETERMINISM_HPP

#include <cstdio>

namespace vm {
	
	namespace determinism {
		typedef size_t deterministic_timestamp;
		enum simulationMode {REALTIME = 0, DETERMINISTIC1 = 1, DETERMINISTIC2 = 2};

		void setDeterministicMode(simulationMode mode);
		//bool isInDeterministicMode();
		bool canCompute();
		void checkAndWaitUntilCanCompute();
		bool isComputing();

		void startComputation(deterministic_timestamp ts, int d);
		void endComputation(bool hasWork);

		deterministic_timestamp getCurrentLocalTime();
		void incrCurrentLocalTime(deterministic_timestamp time);
		void setCurrentLocalTime(deterministic_timestamp time);
	}
}
#endif
