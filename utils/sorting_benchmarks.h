/*
 *  Sorting Benchmarks
 *  Author:		Patrick Flick
 *  Version:	0.1
 *  Description:
 *				This file implements generators as benchmark-inputs for sorting.
 *				The generators generate the 8 benchmarks described in
 *				"A Randomized Parallel Sorting Algorithm with an Experimental Study" by
 *				David R. Helman, David A. Bader and Joseph JaJa. 
 */

#ifndef SORTING_BENCHMARKS_H
#define SORTING_BENCHMARKS_H

#include <cstdlib>
#include <vector>

namespace Benchmark {

class Generator {
	public:
		virtual int operator()()=0;
};

class Uniform : public Generator {
	public:
		int operator()() {
			return rand();
		}
};

class Gaussian : public Generator {
	public:
		int operator()() {
			int r1 = rand();
			int r2 = rand();
			int r3 = rand();
			int r4 = rand();
			return (int)((long long)r1+(long long)r2+(long long)r3+(long long)r4)/4;
		}
		
		Gaussian() {
		}
};

class Zero : public Generator {
	private:
		int val;
	public:
		int operator()() {
			return val;
		}
		
		Zero(int v) : val(v) {}
		
		Zero() : val(0) {}
};

class BuckedSorted : public Generator {
	private:
		long long n;
		unsigned int p; 
		long long counter;
	public:
		int operator()() {
			unsigned int from = counter*((long long)p)*((long long)p)/n % p;
			counter++;
			return (rand() % (RAND_MAX/p)) + (RAND_MAX/p)*from;
		}
		
		BuckedSorted(long long n, unsigned int p) : n(n),p(p),counter(0) {}
};

class gGroup : public Generator {
	private:
		long long n;
		unsigned int p;
		unsigned int g;
		long long c; // counter
	public:
		int operator()() {
			unsigned int q = c*((long long)p)/n; // 0..(p-1)  CPU Index
			unsigned int j = q/g+1; // 1..p/g   Group Index
			unsigned int i = c*((long long)p)*((long long)g)/n % g;
			int from = (((j-1)*g+p/2-1+i) % p + 1);
			if (from == p) from = 0;
			c++;
			return (rand() % (RAND_MAX/p)) + (RAND_MAX/p)*from;
		}
		
		gGroup(long long n, unsigned int p, unsigned int g) : n(n),p(p),g(g),c(0) {}
};

class Staggered : public Generator {
	private:
		long long n;
		unsigned int p; 
		long long counter;
	public:
		int operator()() {
			long long m = n/p;
			unsigned int i = counter*((long long)p)/n + 1;
			unsigned int from;
			if (i <= p/2) {
				from = 2*i-1;
			} else {
				from = 2*i-p-2;
			}
			counter++;
			return (rand() % (RAND_MAX/p)) + from*(RAND_MAX/p);
		}
		
		Staggered(long long n, unsigned int p) : n(n),p(p),counter(0) {}
};


class DeterministicDuplicates : public Generator {
	private:
		long long n;
		long long counter;
		long long loccount;
		long long m;
		
		int log2(long long val) {
			int result = 0;
			while (val >>= 1) ++result;
			return result;
		}
	public:
		int operator()() {
			if (counter == n) {
				counter = 0;
				loccount = 0;
				m = 1;
			}
			if (loccount >= n/2/m) {
				loccount = 0;
				m *= 2;
			}
			int result = log2(n/m);

			loccount++;
			counter++;
			return result;
		}
		
		DeterministicDuplicates(long long n) : n(n),counter(0),loccount(0),m(1) {}
};


class RandomizedDuplicates : public Generator {
	private:
		long long n;
		unsigned int p;
		unsigned int range;
		long long loccount;
		std::vector<unsigned int> T;
		unsigned int S;
		unsigned int k;
		int val;
		
	public:
		int operator()() {
			if (loccount == n/p) {
				
				loccount = 0;
			}
			if (loccount == 0) {
				S = 0;
				k = 0;
				for (unsigned int i = 0; i < range; i++) {
					S += rand() % range;
					T[i] = S;
				}
				val = rand() % range;
			}
			
			if (loccount >= n*T[k]/p/S) {
				k++;
				val = rand() % range;
			}

			loccount++;
			return val;
		}
		
		RandomizedDuplicates(long long n, unsigned int p, unsigned int range) 
			: n(n),p(p),loccount(0),range(range), S(0), k(0) {
			T = std::vector<unsigned int>(range,0);	
		}
};




} // namespace

#endif
