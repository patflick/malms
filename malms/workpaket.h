/*
 *  Workpaket Interface
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				
 */

#ifndef WORKPAKET_H
#define WORKPAKET_H

#include "threadpool/workqueue.h"

/*
 * This is the Interface for the Workpakets, which must implement the () operator,
 * which is called when the Job is executed
 */
class Workpaket : public Scheduler::WorkQueueItem {
	public:
		virtual void operator()()=0;
};

#endif
