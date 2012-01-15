/*
 *  Implements a splitting algorithm.
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				The splitting algorithm implemented in this file is based on
 *				the list selection algorithm from "The Complexity of Selection 
 *				and Ranking in X + Y and Matrices with Sorted Columns" 
 *				by Frederickson and Johnson, published in Journal of Computer
 *				and System Sciences 24 (1982).
 *				
 *				
 */

#ifndef REDUCE_SPLIT_H
#define REDUCE_SPLIT_H



#include<iterator>
#include<cstdlib>
#include<vector>

namespace malms {

namespace Splitting {


template <typename _Value>
struct PartitionElement {
	_Value value;
	int paket;
	bool operator<(const PartitionElement& oval) const {
		return value < oval.value;
	}
};

/*
 * Partitions the sequence [begin,end) according to the pivot value.
 * This is basically the partition routine from the GNU STL Sort, slightly modified.
 */
template<typename _RandomAccessIterator, typename _Tp>
_RandomAccessIterator partitionPivot(_RandomAccessIterator begin,_RandomAccessIterator end, _Tp pivot) {
	while (true) {
		while (*begin < pivot) ++begin;
		--end;
		while (pivot < *end) --end;
		if (!(begin < end)) return begin;
		std::iter_swap(begin, end);
		++begin;
	}
}

/*
 * Implements the quickselect selection algorithm. Partitions the elements in
 * [begin,end) so that the k-th element in sorted order is at the k-th position
 * in the sequence given by [begin,end). The k-th element can be accessed by
 * *(begin+k).
 */
template<typename _RandomAccessIterator, typename _Distance>
void quickselect(_RandomAccessIterator begin, _RandomAccessIterator end, _Distance k) {
	_RandomAccessIterator b = begin;
	while (end - begin > 1) {
		_RandomAccessIterator pivot = begin + (rand() % (end-begin));
		_RandomAccessIterator cut = partitionPivot(begin,end,*pivot);
		if (cut-begin > k) {
			end = cut;
		} else {
			k -= (cut-begin);
			begin = cut;
		}
	}
}


template<typename _RandomAccessIterator,typename _Distance>
void median_split(_RandomAccessIterator* first,_RandomAccessIterator* last,_RandomAccessIterator* current,int num_of_pakets, _Distance& reduce_prefix_size, _Distance& N) {
	typedef typename std::iterator_traits<_RandomAccessIterator>::value_type _ValueType;

	// (1.1) get medians of all sequences
	for (unsigned int j = 0; j < num_of_pakets; j++) {
		if (first[j] != last[j])
			current[j] = first[j] + (last[j]-first[j])/2;
	}
	// (1.2) find weighted partition of the values at the current splitters
	std::vector<PartitionElement<_ValueType> > partitionSeq(num_of_pakets);
	unsigned int c = 0;
	for (unsigned int j = 0; j < num_of_pakets; j++) {
		if (first[j] != last[j]) {
			partitionSeq[c].value = *current[j];
			partitionSeq[c].paket = j;
			c++;
		}
	}
	
	// sort showed to be faster than a weighted partition
	std::sort(partitionSeq.begin(),partitionSeq.begin()+c);
	
	// get weighted median of medians of all sequences
	_Distance weight = 0;
	unsigned int j = 0;
	while (weight < N/2 && j < c) {
		weight += last[partitionSeq[j].paket] - first[partitionSeq[j].paket];
		j++;
	}
	j--;

	// get median value
	unsigned int paket = partitionSeq[j].paket;
	_ValueType medianValue = *current[paket];
	

	// calc new splitters
	for (int j=0;j<num_of_pakets;j++) {
		if (j != paket) {
			if (*current[j] < medianValue) {
				current[j] = std::lower_bound(current[j],last[j],medianValue);
			} else if (medianValue < *current[j]) {
				current[j] = std::lower_bound(first[j], current[j], medianValue);
			}
		}
	}

	// summing elements
	_Distance sum_els = 0;
	for (unsigned int j = 0; j < num_of_pakets; j++) {
		sum_els += current[j] - first[j];
	}

	if (sum_els < reduce_prefix_size) {
		// go in upper half
		memcpy(first,current,num_of_pakets*sizeof(_RandomAccessIterator));
		reduce_prefix_size -= sum_els;
		N -= sum_els;
	} else {
		// go in lower half
		memcpy(last,current,num_of_pakets*sizeof(_RandomAccessIterator));
		N = sum_els;
	}
}

/*
 * Implements the splitting algorithm based on the selection algorithm from
 * Frederickson and Johnson.
 */
template<typename _RandomAccessIterator,typename _Distance>
void reduce_split(_RandomAccessIterator** splitters, int num_of_pakets, int paket_index, _Distance prefix_size) {
	/* typedefs */
	typedef typename std::iterator_traits<_RandomAccessIterator>::value_type _ValueType;

	/* init splitters */
	_RandomAccessIterator first[num_of_pakets];
	_RandomAccessIterator last[num_of_pakets];
	_RandomAccessIterator current[num_of_pakets];
	memcpy(first,splitters[0],num_of_pakets*sizeof(_RandomAccessIterator));
	memcpy(last,splitters[num_of_pakets],num_of_pakets*sizeof(_RandomAccessIterator));
	
	_Distance reduce_prefix_size = prefix_size;
	
	/* init N */
	_Distance N = 0;
	for (int i = 0; i < num_of_pakets; i++) {
		N += last[i] - first[i];
	}
	
	_Distance reduceUntil =  num_of_pakets + 16;
	
	/* Reduce Split */
	while (N > reduceUntil) {
		// reduce
		median_split(first,last,current,num_of_pakets,reduce_prefix_size,N);
	}
	
	/* Find Splitters using Linear Time Selection */
	std::vector<PartitionElement<_ValueType> > partitionSeq(N);
	typedef typename std::vector<PartitionElement<_ValueType> >::iterator parIt;
	parIt partitionit = partitionSeq.begin();
	for (int j = 0; j < num_of_pakets; j++) {
		for (_RandomAccessIterator i = first[j]; i < last[j]; i++) {
			partitionit->value = *i;
			partitionit->paket = j;
			partitionit++;
		}
	}
	
	quickselect(partitionSeq.begin(), partitionSeq.end(), reduce_prefix_size-1);
	
	for (int i = 0; i < N; i++) {
		
		int paket = partitionSeq[i].paket;
		
		if (i < reduce_prefix_size) {
			first[paket]++;
		} else {
			last[paket]--;
		}
	}
	
	/* save results */
	memcpy(splitters[paket_index+1],last,sizeof(_RandomAccessIterator)*num_of_pakets);
}

} // namespace Splitting

} // namespace malms

#endif
