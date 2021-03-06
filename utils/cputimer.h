/*
 *  CPU Timer
 *  Author:		Patrick Flick
 *  Version:	0.1
 */
 
#ifndef CPU_TIMER_H
#define CPU_TIMER_H

#include<time.h>
#include <stdint.h>
//#include<sys/timeb.h>

class CPUTimer {
	
	private:
		uint64_t starttime;
		uint64_t measured_time;
	
		inline long long microsecsonds_ts() {
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME, &ts);
			return (uint64_t)ts.tv_sec * 1000000LL + (uint64_t)ts.tv_nsec / 1000LL;
		}
	public:
		inline void start() {
			starttime = microsecsonds_ts();
		}
		
		inline void stop() {
			measured_time = microsecsonds_ts() - starttime;
		}
		
		// Returns time in microseconds
		uint64_t getTimeMicro() {
			return measured_time;
		}
		
		// returns time in seconds as double
		double getTime() {
			return (double)measured_time/1000000;
		}

};

#endif
