/*
 *  Multiway Merging Algorithm.
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				This implements a multiway merging algorithm using Singlers
 *				losertree.h implementation
 *				
 */

#ifndef LOSER_MULTIWAY_MERGE_H
#define LOSER_MULTIWAY_MERGE_H

#include <vector>
#include <iterator>
#include <algorithm>

//#include "losertree.h" // loser tree implementation from gcc 4.4.6
#include <parallel/losertree.h>

// define name of Losertree according to gcc version
// Test for GXX Version >= 4.5.0
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#define LOSERTREE_CLASS_NAME __gnu_parallel::_LoserTree
#else
#define LOSERTREE_CLASS_NAME __gnu_parallel::LoserTree
#define LOSERTREE_OLD_GCC 1
#endif

#include <string.h> // for memmove

namespace malms {

namespace Merging {

template<typename _ValueType>
struct LessComp {
	bool operator() (_ValueType const & a, _ValueType const & b) {
		return a < b;
	}
};

/*
 * The algorithm keeps a sorted array of one element from each sequence,
 * it copies the smallest element into the output, then gets the
 * next element from the sequence that the copied element originated from.
 * The position in the sorted array for the new element is found using
 * binary search. Once found, all elements in front of that position are copied
 * one step to the left using memmove. This performes well in practice.
 */
template<typename _RandomAccessIterator, typename _OutputIteratorType>
void multiwaymerge(_RandomAccessIterator* lower_splitters, _RandomAccessIterator* upper_splitters, _OutputIteratorType outputIterator, unsigned int num_of_pakets){
	
	typedef typename std::iterator_traits<_RandomAccessIterator>::value_type _ValueType;
	
	unsigned int k = num_of_pakets;
	
	LessComp<_ValueType> comp;
	
	LOSERTREE_CLASS_NAME<false,_ValueType,LessComp<_ValueType> > lt(k, comp);
	
	// find some element
	_ValueType* someElement = NULL;
	bool foundSomeElement = false;
	for (unsigned int i=0;i<k;i++) {
		if (lower_splitters[i] != upper_splitters[i]) {
			someElement = &(*lower_splitters[i]);
			foundSomeElement = true;
		}
	}
	// are there any elements in the sequences?
	if (!foundSomeElement) return;
	
	// fill loser tree
	unsigned int sequences_left = 0;
	for (unsigned int i=0;i<k;i++) {
		if (lower_splitters[i] != upper_splitters[i]) {
			#ifdef LOSERTREE_OLD_GCC
			lt.insert_start(*lower_splitters[i], i, false);
			#else
			lt.__insert_start(*lower_splitters[i], i, false);
			#endif
			sequences_left++;
		} else {
			#ifdef LOSERTREE_OLD_GCC
			lt.insert_start(*someElement, i, true);
			#else
			lt.__insert_start(*someElement, i, true);
			#endif
		}
	}
	
	// init loser tree
	#ifdef LOSERTREE_OLD_GCC
	lt.init();
	#else
	lt.__init();
	#endif
	
	while(sequences_left > 0) {
		#ifdef LOSERTREE_OLD_GCC
		unsigned int min_i = lt.get_min_source();
		#else
		unsigned int min_i = lt.__get_min_source();
		#endif
		*outputIterator = *lower_splitters[min_i];
		outputIterator++;
		lower_splitters[min_i]++;
		if (lower_splitters[min_i] != upper_splitters[min_i]) {
			#ifdef LOSERTREE_OLD_GCC
			lt.delete_min_insert(*lower_splitters[min_i],false);
			#else
			lt.__delete_min_insert(*lower_splitters[min_i],false);
			#endif
		} else {
			#ifdef LOSERTREE_OLD_GCC
			lt.delete_min_insert(*someElement,true);
			#else
			lt.__delete_min_insert(*someElement,true);
			#endif
			sequences_left--;
		}
	}
	
	if (sequences_left>0) {
		// for the last sequence that still contains elements:
		#ifdef LOSERTREE_OLD_GCC
		unsigned int min_i = lt.get_min_source();
		#else
		unsigned int min_i = lt.__get_min_source();
		#endif
		std::copy(lower_splitters[min_i],upper_splitters[min_i],outputIterator);
	}
}

} // namespace Merging

} // namespace malms
#endif
