/*
 *  CPU Timer
 *  Author:		Patrick Flick
 *  Version:	0.1
 */
 
#ifndef CPU_TIMER_H
#define CPU_TIMER_H

#include<sys/time.h>

class CPUTimer {
	
	private:
		long long starttime;
		long long measured_time;
	
		inline long long microsecsonds_since_midnight_GMT() {
			struct timeval t;
			gettimeofday(&t,NULL);
			return((long long)t.tv_sec%(24*3600) * 1000000 + t.tv_usec);
		}
	public:
		inline void start() {
			starttime = microsecsonds_since_midnight_GMT();
		}
		
		inline void stop() {
			measured_time = microsecsonds_since_midnight_GMT() - starttime;
		}
		
		// Returns time in microseconds
		long long getTimeMicro() {
			return measured_time;
		}
		
		// returns time in seconds as double
		double getTime() {
			return (double)measured_time/1000000;
		}

};

#endif
