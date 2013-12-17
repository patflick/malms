/*
 *  Dynamically loads cores according to a given scheme for experimenting with
 *  dynamic load capabilities of the Malleable Scheduler
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				Loads cores according to a given schedule and 
 *  			sends Signals to the Malleable-Scheduler, telling it which cores
 *  			are loaded and not loaded.
 *				Target PID is defined via commandline argument.
 */
 
// defines weather information is send to MALMS or not
#define SENDINFO 1
#define BLOCK_CYCLE_MICROSEC 100000L;
#define ARG_INFO "info"
#define ARG_NOINFO "noinfo"
 
// exec and fork
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <vector>
#include <cstring>
#include <iostream>

// signaling
#include <signal.h>

// nanosleep
#include <time.h>

// scheduler functions for high priority and thread pinning
#include <sched.h>

// threading
#include <boost/thread.hpp>

// GCC Atomics:
// Check for g++ version >= 4.5
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#include <atomic>
#else
// Check for g++ version >= 4.4
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)
#include <cstdatomic>
#else
#error "g++ version < 4.4 is not supported"
// todo add file for atomics for g++ < 4.4
#endif
#endif

// Signal to be waited on before loading cores
#define SIGSTARTBLOCKCORES SIGRTMIN+4

// thread work function memory size per 1000x ints (10000 = 40 MB)
#define THREAD_WORK_MEM 10000
// threads work through tiles in their work mem, size per 1000x ints (25 = 100 kB)
#define TILE_SIZE 25
// number of times each tile gets repeated (lowers mem load through caching)
#define TILE_REPEAT 2


std::vector<boost::thread*> threads;
std::vector<boost::condition_variable*> thread_cd;
std::vector<boost::mutex*> thread_mutex;
std::atomic<bool>* thread_active;
std::atomic<bool>* thread_quit;
std::atomic<int>* thread_work_counter;
int** thread_work_mem;
std::atomic<int> numthreads_sleeping;
std::atomic<bool> quit;
std::atomic<pid_t> pidofsort;
std::atomic<int> p;
std::atomic<bool> send_info;
std::atomic<long long> block_cycle_nanosec;

void pin_to_core(int cpuid) {
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(cpuid,&set);
	sched_setaffinity(0,sizeof(cpu_set_t),&set);
}

void set_max_prio() {
	// set priority to max
	struct sched_param para;
	sched_getparam(0, &para);
	para.sched_priority = sched_get_priority_max(sched_getscheduler(0));
	sched_setparam(0, &para);
}

void block_all_signals() {
	sigset_t sigset;
	sigfillset(&sigset);
	pthread_sigmask(SIG_BLOCK, &sigset, NULL);
}

void dowork_mem(int z, int* work_mem) {
	unsigned int c = 0;
	for (unsigned int i = 0; i < 1000; i++) {
		// memory intensive work
		work_mem[z] = (++c + work_mem[z]) % 13;
		z++;
	}
}

void thread_func(int coreid) {
	block_all_signals();
	pin_to_core(coreid);
	set_max_prio();
	int work_counter = 0;
	int z = 0;
	// waiting till rdy for work
	{ // synchronized
		boost::unique_lock<boost::mutex> lock(*thread_mutex[coreid]);
		numthreads_sleeping++;
		thread_cd[coreid]->wait(lock);
	}
	int* work_mem = thread_work_mem[coreid];
	while (true) {
		if (!thread_active[coreid]) {
			if (thread_quit[coreid]) {
				thread_work_counter[coreid] = work_counter;
				return;
			}
			
			{ // synchronized
				boost::unique_lock<boost::mutex> lock(*thread_mutex[coreid]);
				if (!thread_active[coreid]) {
					thread_cd[coreid]->wait(lock);
				}
			}
			if (thread_quit[coreid]) {
				thread_work_counter[coreid] = work_counter;
				return;
			}
		}else{
			z=((work_counter % TILE_SIZE) + (work_counter / (TILE_SIZE*TILE_REPEAT))*TILE_SIZE)%THREAD_WORK_MEM;
			dowork_mem(1000*z, work_mem);
			work_counter++;
		}
		
	}
}

void loadcore(int coreid, pid_t pid) {
	if (send_info) {
		union sigval value;
		value.sival_int = coreid;
		sigqueue(pid, SIGRTMIN+1, value);
	}
	{
		boost::unique_lock<boost::mutex> lock(*thread_mutex[coreid]);
		thread_active[coreid] = true;
		thread_cd[coreid]->notify_all();
	}
}

void unloadcore(int coreid, pid_t pid) {
	{
		boost::unique_lock<boost::mutex> lock(*thread_mutex[coreid]);
		thread_active[coreid] = false;
	}
	if (send_info) {
		union sigval value;
		value.sival_int = coreid;
		sigqueue(pid, SIGRTMIN+2, value);
	}
}

void waitforpidandexit(pid_t pid) {
	waitpid(pid, NULL, 0);
	exit(0);
}

void handler(int signum) {
	// dont do much :)
}

void controlthreadfunc() {
	set_max_prio();
	// set signal masks
	struct sigaction act;
	act.sa_handler = &handler;
	sigaction(SIGSTARTBLOCKCORES, &act, NULL);
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask,SIGSTARTBLOCKCORES);

	// prepare time struct for sleep
	long sleep_intervall = block_cycle_nanosec;
	struct timespec req;
	req.tv_sec = 0;
	req.tv_nsec = sleep_intervall;
	
	// wait for signal 'Start Block Cores' from timesortfile
	sigsuspend(&mask);
	
	// get pid
	pid_t pid = pidofsort;
	
	// start blocking cores
	loadcore(1, pid);
	loadcore(2, pid);
	//loadcore(4, pid);
	//loadcore(6, pid);
	//nanosleep(&req,NULL);
	
	while (true) {
		
		nanosleep(&req,NULL);
		if (quit) break;
		
	        loadcore(0, pid);
		//loadcore(4, pid);
		//unloadcore(3, pid);
		//unloadcore(7, pid);
		
		nanosleep(&req,NULL);
		if (quit) break;
		
		//loadcore(1, pid);
		//loadcore(5, pid);
		unloadcore(1, pid);
		unloadcore(2, pid);
		
		nanosleep(&req,NULL);
		if (quit) break;
		
		loadcore(1, pid);
		loadcore(2, pid);
		//loadcore(7, pid);
		unloadcore(0, pid);
		//unloadcore(5, pid);
		
		nanosleep(&req,NULL);
		if (quit) break;
		
		//loadcore(3, pid);
		//loadcore(7, pid);
		//unloadcore(2, pid);
		//unloadcore(6, pid);
		//unloadcore(6, pid);
	}
	
	// quit working threads
	// has to be done here to prevent deadlock
	for (unsigned int i = 0; i < p; i++) {
		thread_quit[i] = true;
		if (!thread_active[i]) {
			{
				boost::unique_lock<boost::mutex> lock(*thread_mutex[i]);
				thread_cd[i]->notify_all();
			}
		} else {
			thread_active[i] = false;
		}
	}
}


// usage: ./dynloadcores [info/noinfo] [Cyles in NanoSecs] ./timesortfile [SORT OPTIONS]
int main(int argc, char* argv[]) {
	// read input arguments
	if (strcmp(argv[1],ARG_INFO)==0) {
		send_info = true;
	} else if (strcmp(argv[1],ARG_NOINFO)==0) {
		send_info = false;
	} else {
		std::cout << "Wrong Usage, you are doing it wrong!" << std::endl;
		return 0;
	}
	
	block_cycle_nanosec = atol(argv[2]);
	if (block_cycle_nanosec == 0) {
		std::cout << "Wrong Usage, you are doing it wrong!" << std::endl;
		return 0;
	}
	
	
	// prepare arguments for call of "timesortfile"
	int argc_sort = argc-1;
	char* argv_sort[argc_sort+1];
	argv_sort[argc_sort] = NULL;
	for (int i = 3; i < argc-1; i++) {
		argv_sort[i-3] = argv[i];
	}
	argv_sort[argc_sort-3] = "-p";
	char spid[6];
	sprintf(spid, "%u", getpid());
	argv_sort[argc_sort-2] = spid;
	argv_sort[argc_sort-1] = argv[argc-1];
	
	
	// init thread/synchronization variables
	p = boost::thread::hardware_concurrency();
	threads = std::vector<boost::thread*>(p,NULL);
	thread_cd = std::vector<boost::condition_variable*>(p,NULL);
	thread_mutex = std::vector<boost::mutex*>(p,NULL);
	thread_active = new std::atomic<bool>[p];
	thread_quit = new std::atomic<bool>[p];
	thread_work_counter = new std::atomic<int>[p];
	thread_work_mem = new int*[p];
	numthreads_sleeping = 0;
	quit = false;
	pidofsort = 0;
	// start control thread
	boost::thread control_thread(&controlthreadfunc);
	
	// init mutexes and cond-vars and start threads
	for (unsigned int i = 0; i < p; i++) {
		thread_cd[i] = new boost::condition_variable();
		thread_mutex[i] = new boost::mutex();
		thread_active[i] = false;
		thread_work_counter[i] = 0;
		// create and init thread work mem
		thread_work_mem[i] = new int[1000*THREAD_WORK_MEM];
		for (int j=0;j<1000*THREAD_WORK_MEM;j++) thread_work_mem[i][j]=0;
		
		// start threads
		thread_quit[i] = false;
		threads[i] = new boost::thread(&thread_func, i);
		
	}
	
	block_all_signals();
	
	// wait for all threads to be ready and sleeping
	while (numthreads_sleeping < p) {
		struct timespec req;
		req.tv_sec = 0;
		req.tv_nsec = 100000L;
		nanosleep(&req, NULL);
	}
	

	
	// call timesortfile with arguments provided
	pid_t pid = fork();
	pidofsort = pid;
	if (pid == -1) {
		std::cout << "fork() failed" << std::endl;
		exit(1);
	} else if (pid == 0) {
		execv(argv[3],argv_sort);
		std::cout << "execv() failed" << std::endl;
		exit(1);
	}
	
	// wait for sorting to be finished
	waitpid(pid, NULL, 0);
	
	
	// quit all threads
	quit = true;
	for (unsigned int i = 0; i < p; i++) {
		threads[i]->join();
	}
	control_thread.join();
	
	// get sum of work counters
	unsigned long long aggr_work_counter = 0;
	for (unsigned int i = 0; i < p; i++) {
		aggr_work_counter += thread_work_counter[i];
	}
	// print number of work done
	std::cout << ";" << aggr_work_counter;
	
	return 0;
}
