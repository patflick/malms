/*
 *  Implements the maleable mergesort algorithm.
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				This file implements the maleable multiway mergesort algorithm
 *				that uses the maleable scheduler.
 *				
 *				
 */

#ifndef MALEABLE_MERGESORT_H
#define MALEABLE_MERGESORT_H

#include <algorithm>
#include <vector>
#include<parallel/algorithm>

#include<iterator>

#include "threadpool/maleablescheduler.h"
#include "threadpool/workqueue.h"
#include "workpaket.h"
#include "sort_paket.h"
#include "merge_paket.h"
#include "split_paket.h"
#include "copy_paket.h"


#ifdef TIMING_PHASES
#include <iostream>
#include <string>
#include "../utils/cputimer.h"
#endif

#ifdef TIMING_PHASES_CSV
#include "../utils/cputimer.h"
#include "../utils/timingtable.h"
#endif


namespace malms {

#ifdef TIMING_PHASES
void outputTime(std::string name,double time) {
	std::cout << "        " << name << ": " << time << " s" << std::endl;
}
#endif
#ifdef TIMING_THREADS
void outputThreadTime(std::string name,double time) {
	std::cout << "          " << name << " with Thread " << boost::this_thread::get_id() << ": " << time << " s" << std::endl;
}
#endif

// Returns the size of each paket for any number of pakets and any paket.
template<typename _Distance>
inline _Distance paket_size(_Distance n,unsigned int num_pakets,unsigned int paket) {
	return n/num_pakets + ((paket < n%num_pakets)?1:0);
}



/*
 * The Mergesort function, sorting the sequence given by [begin,end) using
 * num_of_pakets pakets in each step and the workqueue given by queue.
 */
template<typename _RandomAccessIterator>
void sort(_RandomAccessIterator begin,_RandomAccessIterator end, unsigned int num_of_pakets, Scheduler::WorkQueue* queue) {
	typedef typename std::iterator_traits<_RandomAccessIterator>::difference_type _Distance;
	typedef typename std::iterator_traits<_RandomAccessIterator>::value_type _ValueType;	
	
	#ifdef TIMING_PHASES
	CPUTimer timer;
	timer.start();
	#endif
	#ifdef TIMING_PHASES_CSV
	CPUTimer timer;
	timer.start();
	#endif
	// get size
	_Distance n = end - begin;
	
	// array for buffering
	std::vector<_ValueType*> buffers(num_of_pakets);
	
	
	// create pakets for run formation
	typedef typename std::vector<_ValueType*>::iterator _bufIt;
	
	_RandomAccessIterator begin_i = begin;
	for (unsigned int i = 0; i < num_of_pakets; i++) {
		queue->push(new SortPaket<_RandomAccessIterator,_bufIt>(begin_i,begin_i+paket_size(n,num_of_pakets,i),buffers.begin()+i));
		begin_i = begin_i + paket_size(n,num_of_pakets,i);
	}
	// wait until all pakets are done
	queue->blockuntildone();
	
	#ifdef TIMING_PHASES
	timer.stop();
	outputTime("Sorting Phase",timer.getTime());
	timer.start();
	#endif
	
	#ifdef TIMING_PHASES_CSV
	timer.stop();
	TimingTable::addValue("Sorting",timer.getTime());
	timer.start();
	#endif
	
	/* initialize for parallel splitting */
	
	_ValueType*** splitters =  new _ValueType**[(num_of_pakets+1)];
	for (unsigned int i=0; i < num_of_pakets+1;i++) {
		splitters[i] = new _ValueType*[num_of_pakets];
	}
	
	begin_i = begin;
	_Distance sumsizes[num_of_pakets];
	for (unsigned int i = 0; i < num_of_pakets;i++) {
		// init upper and lower splitters with begin and ends of the
		// sorted sequences
		splitters[0][i] = buffers[i];
		begin_i = begin_i + paket_size(n,num_of_pakets,i);
		splitters[num_of_pakets][i] = buffers[i] + paket_size(n,num_of_pakets,i);
		// init prefix sum of paket sizes
		sumsizes[i] = begin_i - begin;
	}
	
	for (unsigned int i = 0; i < num_of_pakets-1; i++) {
		queue->push(new SplitPaket<_ValueType*>(splitters, num_of_pakets, i, sumsizes[i]));
	}
	
	queue->blockuntildone();
	
	
	#ifdef TIMING_PHASES
	timer.stop();
	outputTime("Split",timer.getTime());
	timer.start();
	#endif
	
	#ifdef TIMING_PHASES_CSV
	timer.stop();
	TimingTable::addValue("Splitting",timer.getTime());
	timer.start();
	#endif

	// merge in buffer (not possible inplace, due to parallelism)
	/*
	_ValueType* buffer = static_cast<_ValueType*>(::operator new(sizeof(_ValueType) * n));	
	
	_ValueType* buffer_curPos = buffer;
	
	for (unsigned int i=0;i<num_of_pakets;i++) {
		queue->push(new MergePaket<_RandomAccessIterator,_ValueType*>(splitters[i],splitters[i+1],buffer_curPos,num_of_pakets));
		buffer_curPos += paket_size(n,num_of_pakets,i);
	}
	* */

	_RandomAccessIterator buffer_curPos = begin;

	for (unsigned int i=0;i<num_of_pakets;i++) {
		queue->push(new MergePaket<_ValueType*,_RandomAccessIterator>(splitters[i],splitters[i+1],buffer_curPos,num_of_pakets));
		buffer_curPos += paket_size(n,num_of_pakets,i);
	}

	queue->blockuntildone();
	
	
	#ifdef TIMING_PHASES
	timer.stop();
	outputTime("Merge",timer.getTime());
	timer.start();
	#endif
	
	#ifdef TIMING_PHASES_CSV
	timer.stop();
	TimingTable::addValue("Merging",timer.getTime());
	#endif
	
	// copy back
	/*
	buffer_curPos = buffer;
	begin_i = begin;
	for (unsigned int i=0;i<num_of_pakets;i++) {
		queue->push(new CopyPaket<_ValueType*,_RandomAccessIterator>(buffer_curPos,buffer_curPos+paket_size(n,num_of_pakets,i),begin_i));
		buffer_curPos += paket_size(n,num_of_pakets,i);
		begin_i += paket_size(n,num_of_pakets,i);
	}

	
	queue->blockuntildone();
	*/
	
	#ifdef TIMING_PHASES
	timer.stop();
	outputTime("Copy",timer.getTime());
	timer.start();
	#endif
	
	// clean up
	for (unsigned int i = 0; i < num_of_pakets;i++) {
		delete [] splitters[i];
		delete [] buffers[i];
	}
	delete [] splitters[num_of_pakets];
	delete [] splitters;
	//delete buffer;
	
	#ifdef TIMING_PHASES
	timer.stop();
	outputTime("Cleanup",timer.getTime());
	#endif
}

} // namespace

#endif
