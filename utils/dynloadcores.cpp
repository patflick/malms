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
 
// exec and fork
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

#include <cstring>
#include <iostream>

// signaling
#include <signal.h>

#define SIGSTARTBLOCKCORES SIGRTMIN+4



void handler(int signum) {
	// dont do much :)
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
	
	// set signal masks
	struct sigaction act;
	act.sa_handler = &handler;
	sigaction(SIGSTARTBLOCKCORES, &act, NULL);
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask,SIGSTARTBLOCKCORES);
	
	// call timesortfile with arguments provided
	pid_t pid = fork();
	if (pid == -1) {
		std::cout << "fork() failed" << std::endl;
		exit(1);
	} else if (pid == 0) {
		execv(argv[1],argv_sort);
		std::cout << "execv() failed" << std::endl;
		exit(1);
	}
	
	// wait for signal 'Start Block Cores' from timesortfile
	sigsuspend(&mask);
	
	// start blocking cores
	
	
	
}
