# Makefile for Maleable-Mergesort Testing and Timing
LIBS = -lboost_thread-mt -fopenmp
SORT_LIB = $(wildcard ../malms/*.cpp ../malms/*.h)
UTILS_LIB = $(wildcard ../utils/*.cpp ../utils/*.h)
OPTIMIZATION_LVL = -O2
CC = g++
		
all: testmergesort
		
# testing for correctness of library sort function (no optimization)
testmergesort: testmergesort.cpp $(SORT_LIB)
		$(CC) testmergesort.cpp -o testmergesort $(LIBS) -g
	
clean:
	rm -f testmergesort
