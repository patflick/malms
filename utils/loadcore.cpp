#include <cstdlib>
#include <sched.h>
#include <boost/thread.hpp>
#include <iostream>


void endless() {
	while (true);
}


void blockcore(int cpuid) {
	// set cpu affinity for thread - linux style
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(cpuid,&set);
	sched_setaffinity(0,sizeof(cpu_set_t),&set);
	
	// set priority to max
	struct sched_param para;
	sched_getparam(0, &para);
	para.sched_priority = sched_get_priority_max(sched_getscheduler(0));
	sched_setparam(0, &para);
	
	endless();
}


int main(int argc, char* argv[]) {
	int blockcores = 1;
	if (argc > 1) {
		blockcores = atoi(argv[1]);
	}
	
	int lastcore = boost::thread::hardware_concurrency()-1;
	
	while (blockcores > 1 && lastcore > 0) {
		boost::thread t(&blockcore,lastcore);
		lastcore--;
		blockcores--;
	}
	
	if (blockcores > 0) {
		blockcore(lastcore);
	}
	
	return 0;
}
