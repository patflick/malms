# Makefile for Maleable-Mergesort Utils
LIBS = -lboost_thread -lboost_system -fopenmp
FLAGS = -O2
CC = g++

all: generatesortinput sendblockcore waitforsignal loadcore

generatesortinput: generatesortinput.cpp sorting_benchmarks.h
		$(CC) generatesortinput.cpp -o generatesortinput $(FLAGS)
sendblockcore: sendblockcore.cpp
		$(CC) sendblockcore.cpp -o sendblockcore $(FLAGS)
waitforsignal: waitforsignal.cpp
		$(CC) waitforsignal.cpp -o waitforsignal $(FLAGS)
loadcore: loadcore.cpp
		$(CC) loadcore.cpp -o loadcore $(LIBS) -O0
		
clean:
		rm -f generatesortinput sendblockcore waitforsignal loadcore
