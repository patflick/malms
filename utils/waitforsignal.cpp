/*
 *  Waiting for Signal to Start the core disableing/enableing
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				Waits for the SIGSTARTBLOCKCORE Signal from the sorting
 *				routine, and just quits when received.
 */

#include <signal.h>
#include <unistd.h>

#define SIGSTARTBLOCKCORES SIGRTMIN+4

void handler(int signum) {
	// dont do much :)
}

int main() {
	struct sigaction act;
	act.sa_handler = &handler;
	sigaction(SIGSTARTBLOCKCORES, &act, NULL);
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask,SIGSTARTBLOCKCORES);
	sigsuspend(&mask);
	return 0;
}
