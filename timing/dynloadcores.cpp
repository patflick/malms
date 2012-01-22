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


std::vector<boost::thread*> threads;
std::vector<boost::condition_variable*> thread_cd;
std::vector<boost::mutex*> thread_mutex;
std::atomic<bool>* thread_active;
std::atomic<bool>* thread_quit;
std::atomic<int>* thread_work_counter;
std::atomic<int> numthreads_sleeping;
std::atomic<bool> quit;
std::atomic<pid_t> pidofsort;

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

void dowork() {
	unsigned int c = 0;
	for (unsigned int i = 0; i < 1000; i++) {
		// non memory intensive work
		// cannot be optimized by compiler
		// c = ++c % 13;
	}
}

void thread_func(int coreid) {
	block_all_signals();
	pin_to_core(coreid);
	set_max_prio();
	int work_counter = 0;
	// waiting till rdy for work
	{ // synchronized
		boost::unique_lock<boost::mutex> lock(*thread_mutex[coreid]);
		numthreads_sleeping++;
		thread_cd[coreid]->wait(lock);
	}
	while (true) {
		dowork();
		work_counter++;
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
		}
		
	}
}

void loadcore(int coreid, pid_t pid) {
	#ifdef SENDINFO
	union sigval value;
	value.sival_int = coreid;
	sigqueue(pid, SIGRTMIN+1, value);
	#endif
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
	#ifdef SENDINFO
	union sigval value;
	value.sival_int = coreid;
	sigqueue(pid, SIGRTMIN+2, value);
	#endif
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
	long sleep_intervall = BLOCK_CYCLE_MICROSEC;
	struct timespec req;
	req.tv_sec = 0;
	req.tv_nsec = sleep_intervall;

	// wait for signal 'Start Block Cores' from timesortfile
	sigsuspend(&mask);
	
	// get pid
	pid_t pid = pidofsort;
	
	
	// start blocking cores
	loadcore(0, pid);
	//loadcore(1, pid);
	
	while (true) {
		loadcore(1, pid);
		nanosleep(&req,NULL);
		//if (quit) return;
		loadcore(2, pid);
		unloadcore(1, pid);
		nanosleep(&req,NULL);
		//if (quit) return;
		unloadcore(0, pid);
		loadcore(3, pid);
		loadcore(1, pid);
		nanosleep(&req,NULL);
		//if (quit) return;
		unloadcore(3, pid);
		unloadcore(2, pid);
		nanosleep(&req,NULL);
		if (quit) return;
	}
	
}

int main(int argc, char* argv[]) {
	// prepare arguments for call of "timesortfile"
	char* argv_sort[argc+2];
	argv_sort[argc+1] = NULL;
	for (int i = 1; i < argc-1; i++) {
		argv_sort[i-1] = argv[i];
	}
	argv_sort[argc-2] = "-p";
	char spid[6];
	sprintf(spid, "%u", getpid()); 
	argv_sort[argc-1] = spid;
	argv_sort[argc] = argv[argc-1];
	
	/*
	// echo sort parameters
	for (int i = 0; i < argc+2; i++) {
		std::cout << argv_sort[i] << " ";
	}
	std::cout << std::endl;
	*/

	
	// init thread/synchronization variables
	int p = boost::thread::hardware_concurrency();
	threads = std::vector<boost::thread*>(p,NULL);
	thread_cd = std::vector<boost::condition_variable*>(p,NULL);
	thread_mutex = std::vector<boost::mutex*>(p,NULL);
	thread_active = new std::atomic<bool>[p];
	thread_quit = new std::atomic<bool>[p];
	thread_work_counter = new std::atomic<int>[p];
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
	//pid_t pid = 1;
	pid_t pid = fork();
	pidofsort = pid;
	if (pid == -1) {
		std::cout << "fork() failed" << std::endl;
		exit(1);
	} else if (pid == 0) {
		execv(argv[1],argv_sort);
		std::cout << "execv() failed" << std::endl;
		exit(1);
	}
	
	// wait for sorting to be finished
	waitpid(pid, NULL, 0);
	
	// quit all threads
	quit = true;
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