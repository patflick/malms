#include <iostream>
#include "cputimer.h"

int main() {
	CPUTimer timer;
	timer.start();
	unsigned int c = 0;
	for (unsigned int i = 0; i < 1000; i++) {
		// loop
		c = ++c % 13;
	}
	timer.stop();
	std::cout << "Loop elapsed time: " << timer.getTime() << " c=" << c << std::endl;
}
