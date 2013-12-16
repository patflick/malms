# Makefile for Maleable-Mergesort Testing and Timing
LIBS = -lboost_thread -lboost_system -fopenmp -ltbb
SORT_LIB = $(wildcard ../malms/*.cpp ../malms/*.h)
UTILS_LIB = $(wildcard ../utils/*.cpp ../utils/*.h)
OPTIMIZATION_LVL = -O2
CC = g++
		
all: timesortfile dynloadcores
		
# timing via data input and core blocking
timesortfile: timesortfile.cpp $(SORT_LIB) $(UTILS_LIB)
		cd ../utils; make all; cd ../timing
		$(CC) timesortfile.cpp -o timesortfile $(LIBS) $(OPTIMIZATION_LVL)
		
# timing via data input and core blocking
timesortfile_phases: timesortfile.cpp $(SORT_LIB) $(UTILS_LIB)
		cd ../utils; make all; cd ../timing
		$(CC) timesortfile.cpp -o timesortfile $(LIBS) $(OPTIMIZATION_LVL) -DTIMING_PHASES

dynloadcores: timesortfile dynloadcores.cpp $(SORT_LIB) $(UTILS_LIB)
		cd ../utils; make all; cd ../timing
		$(CC) dynloadcores.cpp -o dynloadcores $(LIBS) -std=c++0x $(OPTIMIZATION_LVL)

clean:
	cd ../utils; make clean; cd ../timing
	rm -f timesortfile input.data dynloadcores
