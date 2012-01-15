/*
 *  Workpaket for splitting.
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				Implements the class SplitPaket which implements the Workpaket
 *				Interface.
 *				
 */

#ifndef SPLIT_PAKET_H
#define SPLIT_PAKET_H

#include "workpaket.h"
#include "median_split.h"

namespace malms {

/*
 * Implements the Workpaket Interface. The constructor takes the parameters for
 * splitting and saves them into the class attributes. The () operator excutes
 * the splitting routine.
 */
template<typename _RandomAccessIterator>
class SplitPaket : public Workpaket {
	private:
		// typedefs
		typedef typename std::iterator_traits<_RandomAccessIterator>::difference_type _Distance;
		
		// attributes
		_RandomAccessIterator** splitters;
		int num_of_pakets;
		int paket;
		_Distance prefix_size;
		
	public:
		/*
		 * Searches for num_of_pakets ^2  splitters, so that all elements in the
		 * lower halves of the sequences are smaller or equal to elements in the
		 * upper halves of the sequences. Splitters are returned so that the
		 * number of elements in lower halves are equal to prefix_size.
		 */
		void operator()() {
			//Splitting::parallel_binary_split(splitters, num_of_pakets, paket, prefix_size);
			Splitting::reduce_split(splitters, num_of_pakets, paket, prefix_size);
		}
		
		/*
		 * Constructor initializes the splitting attributes.	
		 */
		SplitPaket(_RandomAccessIterator** splitters, int num_of_pakets, int paket, _Distance prefix_size) {
			this->splitters = splitters;
			this->num_of_pakets = num_of_pakets;
			this->paket = paket;
			this->prefix_size = prefix_size;
		}
};


} // namespace

#endif
