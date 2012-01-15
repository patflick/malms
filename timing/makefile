# Makefile for Maleable-Mergesort Testing and Timing
LIBS = -lboost_thread-mt -fopenmp
SORT_LIB = $(wildcard ../malms/*.cpp ../malms/*.h)
UTILS_LIB = $(wildcard ../utils/*.cpp ../utils/*.h)
OPTIMIZATION_LVL = -O2
CC = g++
		
all: timesortfile
		
# timing via data input and core blocking
timesortfile:  timesortfile.cpp $(SORT_LIB) $(UTILS_LIB)
		cd ../utils; make all; cd ..
		$(CC) timesortfile.cpp -o timesortfile $(LIBS) $(OPTIMIZATION_LVL)
	
clean:
	cd ../utils; make clean; cd ../timing
	rm -f timesortfile input.data
