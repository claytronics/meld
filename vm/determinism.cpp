#include "vm/determinism.hpp"
#include "api/api.hpp"

#define OPINTMOD_CYCLES
#define OPINTDIV_CYCLES
#define OPFLOATDIV_CYCLES
#define ALLOC_CYCLES
#define OPFLOATMUL_CYCLES
#define OPFLOATADD_CYCLES
#define OPFLOATSUB_CYCLES
#define OPFLOATLTE_CYCLES
#define OPFLOATEQ_CYCLES
#define OPFLOATGTE_CYCLES
#define OPFLOATMOD_CYCLES
#define OPINTMUL_CYCLES
#define OPFLOATGT_CYCLES
#define OPFLOATLT_CYCLES
#define OPFLOATNEQ_CYCLES
#define OPINTEQ_CYCLES
#define OPINTLTE_CYCLES
#define OPINTGTE_CYCLES
#define OPINTLT_CYCLES
#define OPINTGT_CYCLES
#define OPINTNEQ_CYCLES
#define OPINTADD_CYCLES
#define OPINTSUB_CYCLES
#define MOVE_CYCLES

using namespace std;

namespace vm {
	
	namespace determinism {
	
		static deterministic_timestamp currentLocalTime = 0;
		static deterministic_timestamp currentComputationEndTime = 0;
		//static simulationMode mode = REALTIME;
		static simulationMode mode = DETERMINISTIC1;
		static bool computing = false;

		void setDeterministicMode(simulationMode m) {
			mode = m;
		}

		bool canCompute() {
				return (mode == REALTIME) || (currentLocalTime < currentComputationEndTime);
		}
		
		void checkAndWaitUntilCanCompute() {
			if(!canCompute()) {
				cout << "can not compute" << endl;
				if(currentComputationEndTime != 0)
					endComputation(true);
				while(!canCompute()) {
					api::pollAndProcess(NULL,NULL);
					usleep(5000); // to avoid polling to much
				}
			cout << "can compute again" << endl;
			}
		}
		
		bool isComputing() {
			return computing;
		}
		
		void startComputation(deterministic_timestamp ts, int d) {
			computing = true;
			currentComputationEndTime = ts + d;
			cout << "start computation till " << currentComputationEndTime <<endl;
		}
		
		void endComputation(bool hasWork) {
			if (!computing) {
				return;
			}
			computing = false;
			cout << "end computation at "<< currentLocalTime;
			setCurrentLocalTime(currentComputationEndTime);
			cout << " ajusted to " << currentLocalTime << endl;
			cout << "api::endComputation..." << endl;
			api::endComputation(NULL, hasWork);
			cout << "ok" << endl;
		}
		
		deterministic_timestamp getCurrentLocalTime() {
			return currentLocalTime;
		}
		
		void incrCurrentLocalTime(deterministic_timestamp time) {
				currentLocalTime += time;
		}
		
		void setCurrentLocalTime(deterministic_timestamp time) {
			if (mode == DETERMINISTIC1) {
				currentLocalTime = max(currentLocalTime, time);
			}
		}
	}
}
