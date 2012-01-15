#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>

// algorithm to test
#include "../malms/threadpool_mergesort.h"
#include "../malms/threadpool/maleablescheduler.h"



#define INPUT_RANDOM_INT 1
#define INPUT_SORTED_INT 2
#define INPUT_SAME_INT 3
#define INPUT_REV_SORTED_INT 4


int testcase = 0;
int errors = 0;



struct Testdata {
	int value;
	long long pos;
	bool operator< (const Testdata& t) const {
		return value < t.value;
	}
	bool operator == (const Testdata& t) const {
		return value == t.value;
	}
};

struct TestdataComp {
	bool operator() (const Testdata& t1,const Testdata& t2) {
		return t1.value < t2.value;
	}
} ;

// testing also if output is permutation of input
void test2(long long size,int cores, int workpakets, int type) {
	std::cout << "Testcase # " << ++testcase << ": [Size: " << size << ", Cores: " << cores << ", Workpakets: " << workpakets << ", Type: ";
	
	std::vector<Testdata> input(size);
	std::vector<Testdata> correct(size);
	
	if (type == INPUT_RANDOM_INT) {
		long long j = 0;
		for (std::vector<Testdata>::iterator i = input.begin();i != input.end();i++) {
			(*i).pos = j++;
			(*i).value = rand();
		}
		std::cout << "Struct Random Ints] ";
	} else if (type == INPUT_SORTED_INT) {
		long long j = 0;
		for (std::vector<Testdata>::iterator i = input.begin();i != input.end();i++) {
			(*i).pos = j++;
			(*i).value = 2*j;
		}
		std::cout << "Struct Sorted Ints] ";
	} else if (type == INPUT_SAME_INT) {
		long long j = 0;
		for (std::vector<Testdata>::iterator i = input.begin();i != input.end();i++) {
			(*i).pos = j++;
			(*i).value = 5;
		}
		std::cout << "Struct All the Same] ";
	}
	std::cout.flush();
	
	// get correct output, assuming that std::sort is correct
	std::copy(input.begin(),input.end(),correct.begin());
	std::sort(correct.begin(),correct.end());
	
	// call mymergesort
	Scheduler::MaleableScheduler* sched = Scheduler::MaleableScheduler::singleton();
	Scheduler::WorkQueue* queue = sched->newJob();
	sched->scheduleToFirst(queue, cores);
	malms::sort(input.begin(),input.end(),workpakets,queue);
	//sched->deleteJob(queue);
	Scheduler::MaleableScheduler::deleteSingleton();
	
	// check if values are the same
	bool c = std::equal(input.begin(),input.end(),correct.begin());
	
	if (!c) {
		std::cout << "\t\tFAIL" << std::endl;
		errors++;
		return;
	}
	
	// check if all elements are in the output sequence by checking the position field
	std::vector<bool> exist(size,false);
	for (std::vector<Testdata>::iterator i = input.begin();i != input.end();i++) {
		if (exist[(*i).pos]) {
			// one elements was in there twice!! :o
			std::cout << "\t\tFAIL" << std::endl;
			errors++;
			return;
		} else {
			exist[(*i).pos] = true;
		}
	}

	
	if (std::find(exist.begin(),exist.end(),false) != exist.end()) {
		// one element was left out!! :o
		std::cout << "\t\tFAIL" << std::endl;
		errors++;
		return;
	}
	
	// nothing failed => This was a triump. I'm making a note here: HUGE SUCCESS.
	std::cout << "\t\tOK" << std::endl;
}


	

void test(long long size, int cores, int workpakets, int type) {
	std::cout << "Testcase # " << ++testcase << ": [Size: " << size << ", Cores: " << cores << ", Workpakets: " << workpakets << ", Type: ";
	

	std::vector<int> input(size);
	std::vector<int> correct(size);
	
	if (type == INPUT_RANDOM_INT) {
		std::generate(input.begin(),input.end(),rand);
		std::cout << "Random Ints] ";
	} else if (type == INPUT_SORTED_INT) {
		long long j = 0;
		for (std::vector<int>::iterator i = input.begin();i != input.end();i++) {
			*i = ++j;
		}
		std::cout << "Sorted Ints] ";
	} else if (type == INPUT_REV_SORTED_INT) {
		long long j = size;
		for (std::vector<int>::iterator i = input.begin();i != input.end();i++) {
			*i = j--;
		}
		std::cout << "Reverse Sorted Ints] ";
	} else if (type == INPUT_SAME_INT) {
		// TODO
		std::cout << "All the Same] ";
	}
	std::cout.flush();
	
	// get correct output, assuming that std::sort is correct
	std::copy(input.begin(),input.end(),correct.begin());
	std::sort(correct.begin(),correct.end());
	
	// call mymergesort
	Scheduler::MaleableScheduler * sched = Scheduler::MaleableScheduler::singleton();
	Scheduler::WorkQueue* queue = sched->newJob();
	sched->scheduleToFirst(queue, cores);
	malms::sort(input.begin(),input.end(),workpakets,queue);
	//sched->deleteJob(queue);
	Scheduler::MaleableScheduler::deleteSingleton();
	// call mcstl
	//__gnu_parallel::parallel_sort_mwms<true,true>(input.begin(),input.end(),std::less<int>(),workpakets); 
	
	// check if sequences are the same
	bool c = std::equal(input.begin(),input.end(),correct.begin());
	if (c) {
		std::cout << "\t\tOK" << std::endl;
	} else {
		std::cout << "\t\tFAIL" << std::endl;
		errors++;
	}
}

int main() {
	test(1000,1,4,INPUT_RANDOM_INT);
	
	// small sequential tests
	test(0,1,1,INPUT_RANDOM_INT);
	test(1,1,1,INPUT_RANDOM_INT);
	test(10,1,1,INPUT_RANDOM_INT);
	test(250,1,1,INPUT_RANDOM_INT);
	
	// sequential with multiple workpakets
	test(1,1,2,INPUT_RANDOM_INT);
	
	test(10,1,513,INPUT_RANDOM_INT);
	
	test(250,1,100,INPUT_RANDOM_INT);
	
	// parallel with same amount of workpakets
	test(1,4,4,INPUT_RANDOM_INT);
	test(100,100,100,INPUT_RANDOM_INT);
	test(10,2,2,INPUT_RANDOM_INT);
	test(13,7,7,INPUT_RANDOM_INT);
	test(250,8,8,INPUT_RANDOM_INT);
	
	// parallel with different amount of workpakets
	test(0,4,13,INPUT_RANDOM_INT);
	test(13,3,18,INPUT_RANDOM_INT);
	test(200,8,7,INPUT_RANDOM_INT);
	test(1000,5,1,INPUT_RANDOM_INT);
	test(137,13,7,INPUT_RANDOM_INT);
	
	// huge tests
	test(10000000,4,4,INPUT_RANDOM_INT);
	test(3000000,8,400,INPUT_RANDOM_INT);
	test(500000,120,240,INPUT_RANDOM_INT);
	
	test(100000000,3,100,INPUT_RANDOM_INT);
	
	// test sorted and reverse sorted
	test(1000,2,2,INPUT_SORTED_INT);
	test(1000,2,2,INPUT_REV_SORTED_INT);
	test(50000,4,4,INPUT_REV_SORTED_INT);
	
	
	// test permu
	test2(10000000,4,4,INPUT_RANDOM_INT);
	test2(10000000,4,4,INPUT_SORTED_INT);
	test2(250,1,4,INPUT_RANDOM_INT);
	test2(250,1,1,INPUT_RANDOM_INT);
	test2(250,1,1,INPUT_SAME_INT);
	test2(250,1,3,INPUT_SAME_INT);
	
	
	// output statistics
	if (errors == 0) {	
		std::cout << "This was a triumph.\nI'm making a note here:\nHUGE SUCCESS." << std::endl;
		std::cout << testcase << " Tests Completed Successfully!" << std::endl;
	} else {
		std::cout << "The Cake is a Lie!!!" << std::endl; 
		std::cout << "[**Error**]  " << testcase << " Tests Completed with " << errors << " Errors." << std::endl;
	}
	
	return 0;

}
