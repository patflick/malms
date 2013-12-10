/*
 *  Send Signals to Maleable-Scheduler
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *  			Sends Signals to Maleable-Scheduler, telling it which cores
 *  			are enabled and disabled.
 *				Target PID and Cores are defined by commandline arguments.
 */

#include <signal.h>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <vector>

#define BLOCK "block"
#define UNBLOCK "unblock"
#define PID "-p"
#define CORES "-c"


void printUsage() {
	std::cout << "Usage:" << std::endl;
	std::cout << "Sending Block Signal:\n\tsendblockcore block -p [PID] -c [CORES]" << std::endl;
	std::cout << "Sending Unblock Signal:\n\tsendblockcore unblock -p [PID] -c [CORES]" << std::endl;
	std::cout << "Where:\n[PID]\t\tProcess-ID pid of the target process." << std::endl;
	std::cout << "[CORES]\t\tCores to be blocked/unblocked, a list of integers:\n\t\t1 3 5" << std::endl;
}


int main(int argc, char* argv[]) {
	/*
	 *  Read Input from argv.
	 */
	
	if (argc < 6) {
		printUsage();
		return 0;
	}
	bool doBlock; // true: block core, false: unblock core
	if (strcmp(argv[1],BLOCK)==0) {
		doBlock = true;
	} else if (strcmp(argv[1],UNBLOCK)==0) {
		doBlock = false;
	} else {
		printUsage();
		return 0;
	}
	
	int pid;
	if (strcmp(argv[2],PID)==0) {
		pid = atoi(argv[3]);
		if (pid == 0) {
			printUsage();
			return 0;
		}
	} else {
		printUsage();
		return 0;
	}


	std::vector<int> inputcores(argc-5);
	if (strcmp(argv[4],CORES)==0) {
		for (unsigned int i = 0; i < inputcores.size(); i++) {
			inputcores[i] = atoi(argv[5+i]);
			if (inputcores[i] == 0) {
				printUsage();
				return 0;
			}
		}
	} else {
		printUsage();
		return 0;
	}
	
	/*
	 * Send Signals
	 */
	for (unsigned int i = 0; i < inputcores.size(); i++) {
		union sigval value;
		value.sival_int = inputcores[i];
		if (doBlock) {
			sigqueue(pid, SIGRTMIN+1, value);
		} else {
			sigqueue(pid, SIGRTMIN+2, value);
		}
	}
	return 0;
}
