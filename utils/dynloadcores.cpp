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

#include <cstring>
#include <iostream>

// signaling
#include <signal.h>

int main(int argc, char* argv[]) {


}
