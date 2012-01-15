/*
 *  Sorting Algorithm Input Generator
 *
 *  Author:		Patrick Flick
 *  Version:	0.1
 *
 *  Description:
 *				Generates Files from the Benchmark Input Generators.
 *				Distrubution and Size of Input are defined by command line arguments.
 */

#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
// include sorting benchmarks for generating data
#include "sorting_benchmarks.h"

#define ARG_N "-n"
#define ARG_T "-t"
#define ARG_P "-p"
#define ARG_G "-g"

// benchmarks (input types)
#define BM_U "U"
#define BM_G "G"
#define BM_Z "Z"
#define BM_B "B"
#define BM_GG "gG"
#define BM_S "S"
#define BM_DD "DD"
#define BM_RD "RD"

void printUsage() {
	// TODO
	std::cout << "You are doing to wrong!" << std::endl;
}

int main(int argc, char* argv[]) {
	unsigned long long n = 0;
	const char* t = "U"; // Default type
	unsigned int p = 0;
	unsigned int g = 0;
	char* filename = NULL;
	for (int i = 1; i < argc-1; i++) {
		if (strcmp(argv[i],ARG_N)==0) {
			// "-n" Input Size
			++i;
			n = atol(argv[i]);
		} else if (strcmp(argv[i],ARG_T)==0) {
			// "-t" Benchmark-Type
			++i;
			t = argv[i];
		} else if (strcmp(argv[i],ARG_P)==0) {
			++i;
			p = atoi(argv[i]);
		} else if (strcmp(argv[i],ARG_P)==0) {
			++i;
			g = atoi(argv[i]);
		}
	}
	
	if (n == 0) {
		printUsage();
		return 0;
	}
	
	filename = argv[argc-1];
	

	
	Benchmark::Generator* inputGenerator = NULL;
	if (strcmp(t,BM_U)==0) {
		inputGenerator = new Benchmark::Uniform();
	} else if (strcmp(t,BM_G)==0) {
		inputGenerator = new Benchmark::Gaussian();
	} else if (strcmp(t,BM_Z)==0) {
		inputGenerator = new Benchmark::Zero();
	} else if (strcmp(t,BM_B)==0) {
		if (p == 0) {
			printUsage();
			return 0;
		}
		inputGenerator = new Benchmark::BuckedSorted(n,p);
	} else if (strcmp(t,BM_GG)==0) {
		if (p == 0 || g == 0) {
			printUsage();
			return 0;
		}
		inputGenerator = new Benchmark::gGroup(n,p,g);
	} else if (strcmp(t,BM_S)==0) {
		if (p == 0) {
			printUsage();
			return 0;
		}
		inputGenerator = new Benchmark::Staggered(n,p);
	} else if (strcmp(t,BM_DD)==0) {
		inputGenerator = new Benchmark::DeterministicDuplicates(n);
	} else if (strcmp(t,BM_RD)==0) {
		// TODO
	}
	
	// Generate Sorting Input
	int* data = new int[n];
	for (unsigned long long i = 0; i < n; i++) {
		data[i] = (*inputGenerator)();
	}
	
	// save to file
	std::ofstream outputfile;
	outputfile.open(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	if (outputfile.is_open()) {
		outputfile.write(reinterpret_cast<char*>(data),n*sizeof(int));
		outputfile.close();
	} else {
		std::cout << "Unable to open file \"" << filename << "\"" << std::endl;
	}
	delete data;
	
	return 0;
}
