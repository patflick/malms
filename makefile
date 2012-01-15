# Makefile for Maleable-Mergesort Testing and Timing
LIBS = -lboost_thread-mt -fopenmp
SORT_LIB = $(wildcard malms/*.cpp malms/*.h)
UTILS_LIB = $(wildcard utils/*.cpp utils/*.h)
OPTIMIZATION_LVL = -O2
CC = g++
		
all:
	cd timing; make all; cd ..
	cd utils; make all;cd ..
	cd test; make all; cd ..
	
clean:
	cd utils; make clean; cd ..
	cd test; make clean; cd ..
	cd timing; make clean; cd ..
