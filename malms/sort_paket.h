/*
 *  Workpaket for sorting.
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				Implements the class SortPaket which implements the Workpaket
 *				Interface.
 *				
 */

#ifndef SORT_PAKET_H
#define SORT_PAKET_H

#include <vector>
#include <algorithm>
#include "workpaket.h"

namespace malms {

/*
 * Implements the Workpaket Interface. The constructor takes two Random-Access-
 * Iterators (begin and end) and saves them into the status of the SortPaket.
 * The () operator sorts the intervall [begin,end) using GNU-sort.
 */
template<typename _RandomAccessIterator, typename _BufferIterator>
class SortPaket : public Workpaket {
	private:
		// typedefs
		typedef typename std::iterator_traits<_RandomAccessIterator>::value_type _ValueType;
		
		// attributes
		_RandomAccessIterator begin;
		_RandomAccessIterator end;
		_BufferIterator buffer;
		
	public:
		/*
		 * Sorts the intervall [begin,end) using a buffer and GNU-sort.
		 */
		void operator()() {
			// do buffered sort
			*buffer = static_cast<_ValueType*>(::operator new(sizeof(_ValueType) * (end-begin)));
			std::copy(begin,end,*buffer);
			std::sort(*buffer,(*buffer)+(end-begin));
		}
		
		/*
		 * Constructor initializes the sort attributes.	
		 */
		SortPaket(_RandomAccessIterator begin, _RandomAccessIterator end, _BufferIterator buf) {
			this->begin = begin;
			this->end = end;
			this->buffer = buf;
		}
};

} // namespace

#endif
