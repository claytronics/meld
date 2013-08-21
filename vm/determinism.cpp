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
		static deterministic_timestamp currentWorldTime = 0;
		static deterministic_timestamp currentComputationEndTime = 0;
		static simulationMode mode = REALTIME;
		//static simulationMode mode = DETERMINISTIC2;
		static bool computing = false;

		void setDeterministicMode(simulationMode m) {
			mode = m;
			cout << "setDeterminismMode: " << m << endl;
		}
		
		simulationMode getSimulationMode() {
			return mode;
		}

		bool canCompute() {
			switch(mode) {
				case REALTIME:
					return true;
					break;
				case DETERMINISTIC1:
					return true;
					break;
				case DETERMINISTIC2:
					return (currentLocalTime < currentComputationEndTime);
					break;
				default:
					return true;
			}
		}
		
		void checkAndWaitUntilCanCompute() {
			switch(mode) {
				case REALTIME:
					break;
				case DETERMINISTIC1:
					//if (currentLocalTime%50 == 0)
					//	api::timeInfo(NULL);
					break;
				case DETERMINISTIC2:
					if(!canCompute()) {
						if(currentComputationEndTime != 0)
							computationPause();
						while(!canCompute()) {
							api::waitAndProcess(NULL,NULL);
						}
					}
					break;
			}
		}
		
		bool mustQueueMessages() {
			return ((mode == DETERMINISTIC2) && isComputing());
		}
		
		bool isComputing() {
			return computing;
		}
		
		
		void resumeComputation(deterministic_timestamp ts, deterministic_timestamp d) {
			switch(mode) {
				case REALTIME:
					break;
				case DETERMINISTIC1:
					break;
				case DETERMINISTIC2:
					computing = true;
					currentComputationEndTime = currentLocalTime + d;
					cout << "start Computation at "<< currentLocalTime << "till " << currentComputationEndTime << endl;
					break;
				default:
					return;
			}
		}
		
		void computationPause() {
			switch(mode) {
				case REALTIME:
				case DETERMINISTIC1:
					break;
				case DETERMINISTIC2:
					if (computing) {
						computing = false;
						setCurrentLocalTime(currentComputationEndTime);
						api::computationPause();						
						cout << "computationPause" << endl;
					}
					break;
				default:
					break;
			}
		}
		
		void workEnd() {
			switch(mode) {
				case REALTIME:
					break;
				case DETERMINISTIC1:
					break;
				case DETERMINISTIC2:
					if (!computing) { return; }
					computing = false;
					setCurrentLocalTime(currentComputationEndTime);
					cout << "WorkEnd" << endl;
					break;
				default:
					break;
			}
			api::workEnd();
			cout << "WorkEnd sent" << endl;
		}
		
		deterministic_timestamp getCurrentLocalTime() {
			return currentLocalTime;
		}
		
		void incrCurrentLocalTime(deterministic_timestamp time) {
				currentLocalTime += time;
		}
		
		void setCurrentLocalTime(deterministic_timestamp time) {
			switch(mode) {
				case REALTIME:
					break;
				case DETERMINISTIC1:
					currentLocalTime = max(currentLocalTime, time);
					cout << "time adjusted: " << currentLocalTime  << endl;
					break;
				case DETERMINISTIC2:
					currentLocalTime = max(currentLocalTime, time);
					break;
				default:
					break;
			}
		}
		
		deterministic_timestamp getCurrentWorldTime() {
			return currentWorldTime;
		}
		
		void setCurrentWorldTime(deterministic_timestamp time) {
			currentWorldTime = max(currentWorldTime, time);
		}
	}
}
