/*
 *  Workpaket for copying data from one array to another.
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				Implements the class CopyPaket which implements the Workpaket
 *				Interface.
 *				
 */

#ifndef COPY_PAKET_H
#define COPY_PAKET_H

#include <vector>
#include <algorithm>
#include "workpaket.h"


namespace malms {

/*
 * Implements the Workpaket Interface. The constructor takes three Random-Access-
 * Iterators (begin, end and target) and saves them into the status of the CopyPaket.
 * The () operator copies the data in [begin,end) to [target,...)
 */
template<typename _RandomAccessIterator,typename _TargetRandomAccessIterator>
class CopyPaket : public Workpaket {
	private:		
		// attributes
		_RandomAccessIterator source_begin;
		_RandomAccessIterator source_end;
		_TargetRandomAccessIterator target_begin;
		
	public:
		/*
		 * Copies the data from [begin,end) to [target,...) using STL copy.
		 */
		void operator()() {
			// copy
			std::copy(source_begin, source_end, target_begin);
		}
		
		/*
		 * Constructor initializes the copying attributes.	
		 */
		CopyPaket(_RandomAccessIterator begin,_RandomAccessIterator end,_TargetRandomAccessIterator target) {
			this->source_begin = begin;
			this->source_end = end;
			this->target_begin = target;
		}
};

} // namespace

#endif
