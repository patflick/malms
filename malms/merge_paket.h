/*
 *  Workpaket for merging.
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				Implements the class MergePaket which implements the Workpaket
 *				Interface.
 *				
 */


#ifndef MERGE_PAKET_H
#define MERGE_PAKET_H

#include <vector>
#include "workpaket.h"
#include "loser_merge.h"

namespace malms {

/*
 * Implements the Workpaket Interface. The constructor takes the parameters for
 * merging and saves them into the class attributes. The () operator excutes
 * the merging routine.
 */
template<typename _RandomAccessIterator, typename _OutputIterator>
class MergePaket : public Workpaket {
	private:
		typedef typename std::iterator_traits<_RandomAccessIterator>::difference_type _Distance;
		typedef typename std::iterator_traits<_RandomAccessIterator>::value_type _ValueType;
		
		// the number of sequences to be merged
		unsigned int num_of_pakets;
		// the lower and upper splitters for all sequences
		_RandomAccessIterator* lower_splitters;
		_RandomAccessIterator* upper_splitters;
		// the iterator for the output
		_OutputIterator outputIterator;
		
		// use buffered merge or not
		bool buffered;
	public:
		/*
		 * Merges num_of_pakets sorted sequences into one sorted sequence.
		 */
		void operator()() {
			
			if (!buffered) {
			
				// need to copy splitters if not buffered, because they are modified during merge
				_ValueType** tmp_lower_splitters = new _ValueType*[num_of_pakets];
				_ValueType** tmp_upper_splitters = new _ValueType*[num_of_pakets];
				std::copy(lower_splitters, lower_splitters+num_of_pakets, tmp_lower_splitters);
				std::copy(upper_splitters, upper_splitters+num_of_pakets, tmp_upper_splitters);
				
				Merging::multiwaymerge(tmp_lower_splitters, tmp_upper_splitters, outputIterator, num_of_pakets);
				
				delete [] tmp_lower_splitters;
				delete [] tmp_upper_splitters;
			
			} else {
			
				// calculate the number of elements to be merged
				_Distance merge_size = 0;
				for (unsigned int i=0; i<num_of_pakets;i++) {
					merge_size += upper_splitters[i] - lower_splitters[i];
				}
				
				// create buffers for merging
				_ValueType* input_buffer = new _ValueType[merge_size];
				_ValueType** buffer_lower_splitters = new _ValueType*[num_of_pakets];
				_ValueType** buffer_upper_splitters = new _ValueType*[num_of_pakets];
				
				// copy the scattered sequences into one buffer and recalculate the
				// splitters
				_ValueType* cur_splitter_pos = input_buffer;
				for (unsigned int i=0; i<num_of_pakets;i++) {
					buffer_lower_splitters[i] = cur_splitter_pos;
					std::copy(lower_splitters[i],upper_splitters[i],cur_splitter_pos);
					cur_splitter_pos += upper_splitters[i] - lower_splitters[i];
					buffer_upper_splitters[i] = cur_splitter_pos;
				}
				
				// call merge
				Merging::multiwaymerge(buffer_lower_splitters, buffer_upper_splitters, outputIterator, num_of_pakets);
				
				// delete buffers
				delete [] input_buffer;
				delete [] buffer_lower_splitters;
				delete [] buffer_upper_splitters;
			}
		}
		
		/*
		 * Constructor initializes the merge attributes.
		 */
		MergePaket(_RandomAccessIterator* lower_splitters, _RandomAccessIterator* upper_splitters, _OutputIterator _outputIterator, unsigned int num_of_pakets)
			:	num_of_pakets(num_of_pakets),
				lower_splitters(lower_splitters),
				upper_splitters(upper_splitters),
				outputIterator(_outputIterator), buffered(false) {
		}
};


} // namespace
#endif
