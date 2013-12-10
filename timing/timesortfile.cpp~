/*
 *  Sort Input File and Ouput Timing.
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				Reads an input file and sorts it via a specified algorithm.
 *				The input file and the used algorithm are specified via
 *				command line argument.
 */
 
#include <iostream>
#include <fstream>
#include <cstring>

// MCSTL MWMS
#include <parallel/algorithm>
#include <parallel/multiway_mergesort.h>

// Maleable MS
#include "../malms/threadpool_mergesort.h"
#include "../malms/threadpool/maleablescheduler.h"

// signaling
#include <signal.h>

// timing
#include "../utils/cputimer.h"

#define SIGSTARTBLOCKCORES SIGRTMIN+4

#define ARG_SIG_PID "-p"
#define ARG_K "-k"
#define ARG_C "-c"
#define ARG_ALG "-a"
#define ARG_ALG_MCSTL "mcstl"
#define ARG_ALG_MALMS "malms"
#define ARG_ALG_STDSORT "stdsort"

// possible algorithms
enum Algorithm {MCSTL_MWMS, MALMS, STDSORT};


void printUsage() {
	std::cout << "Usage:\n\ttimesortfile [OPTIONS] filename" << std::endl;
	std::cout << "Where [OPTIONS] can be\n-k wp\t\t\t Number of Workpakets (must be provided)" << std::endl;
	std::cout << "-p pid\t\t\tThe PID of the process receiving the signal." << std::endl;
	std::cout << "-a algorithm\tThe Algorithm used, can be one of " << ARG_ALG_MCSTL << ", " 
			  << ARG_ALG_MALMS << " or " << ARG_ALG_STDSORT << std::endl;
}


int main(int argc, char* argv[]) {
	// read input settings from command line arguments
	Algorithm a = MALMS;// Default algorithm
	char* filename = NULL;
	int pid = 0;
	int k = 0;
	int i = 1;
	int c = 0;
	while (i < argc-1) {
		if (strcmp(argv[i],ARG_ALG)==0) {
			// "-a" algorithm
			++i;
			if (strcmp(argv[i],ARG_ALG_MCSTL)==0) {
				a = MCSTL_MWMS;
			} else if (strcmp(argv[i],ARG_ALG_MALMS)==0) {
				a = MALMS;
			} else if (strcmp(argv[i],ARG_ALG_STDSORT)==0) {
				a = STDSORT;
			} else {
				printUsage();
				return 0;
			}
		} else if (strcmp(argv[i],ARG_SIG_PID)==0) {
			// "-p" pid of process to signal
			++i;
			pid = atoi(argv[i]);
		} else if (strcmp(argv[i],ARG_K)==0) {
			// "-k" number of workpakets
			++i;
			k = atoi(argv[i]);
		} else if (strcmp(argv[i],ARG_C)==0) {
			// "-c" number of cores for MALMS
			++i;
			c = atoi(argv[i]);
		}
		++i;
	}
	if (k == 0) {
		printUsage();
		return 0;
	}
	
	filename = argv[i];
	
	// read input file, open with ios::ate so that position is at the end of the file
	// which is needed to determine file size
	int * data;
	std::ifstream inputFile(filename, std::ios::in | std::ios::binary | std::ios::ate);
	
	if (!inputFile.is_open()) {
		std::cout << "Unable to open file" << std::endl;
		return 0;
	}
	// read file
	std::ifstream::pos_type filesize;
	filesize = inputFile.tellg();
	char * chardata = new char[filesize];
	inputFile.seekg(0, std::ios::beg);
	inputFile.read(chardata, filesize);
	inputFile.close();
	
	// cast as int
	data = reinterpret_cast<int*>(chardata);
	unsigned long long n = filesize >> 2;
	

	
	// prepare timing function
	CPUTimer timer;
	
	// start sorting with the correct algorithm
	if (a == MCSTL_MWMS) {
		timer.start();
		// give signal that preparation is done
		if (pid != 0) {
			kill(pid, SIGSTARTBLOCKCORES);
		}
		__gnu_parallel::parallel_sort_mwms<false,true>(data,data+n,std::less<int>(),k); 
		timer.stop();
		
	} else if (a == MALMS) {
		timer.start();
		// prepare threadpool
		Scheduler::MaleableScheduler* sched = Scheduler::MaleableScheduler::singleton();
		Scheduler::WorkQueue* queue = sched->newJob();
		if (c == 0) {
			sched->scheduleToAll(queue);
		} else {
			sched->scheduleToFirst(queue, c);
		}
		// give signal that preparation is done
		if (pid != 0) {
			kill(pid, SIGSTARTBLOCKCORES);
		}
		malms::sort(data,data+n,k,queue);
		timer.stop();
	} else if (a == STDSORT) {
		timer.start();
		// give signal that preparation is done
		if (pid != 0) {
			kill(pid, SIGSTARTBLOCKCORES);
		}
		std::sort(data,data+n);
		timer.stop();
	}

	// output measured time and then exit
	std::cout << timer.getTime();
	std::cout.flush();
	return 0;
}
