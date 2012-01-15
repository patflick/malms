#include <iostream>
#include "maleablescheduler.h"
#include "workqueue.h"

class TestPaket : public Scheduler::WorkQueueItem {
	private:
		int phase;
		int count;
	public:
		void operator()() {
			std::cout << "Thread " << boost::this_thread::get_id() << " TestPaket in Phase " << phase << " and Count " << count << std::endl;
			// do something that takes a while
			int max = 100000000;
			int c = 1;
			for (int i=0;i<max;i++) {
				c += i % 13;
			}
		}
		TestPaket(int phase, int count) : phase(phase),count(count){}
};


int main() {
	Scheduler::MaleableScheduler scheduler;
	Scheduler::WorkQueue* queue = scheduler.newJob();
	int p = 4;
	std::vector<int> cores(p);
	for (int i=0;i<p;i++) {
		cores[i] = i;
	}
	scheduler.scheduleJob(queue,cores);
	int phases = 3;
	int perphase = 10;
	for (int i=0;i<phases;i++) {
		for (int j = 0; j < perphase; j++) {
			std::cout << "Pushing Paket " << j << std::endl;
			queue->push(new TestPaket(i,j));
		}
	}
	std::cout << "Waiting for thread to be done" << std::endl;
	queue->blockuntildone();
	scheduler.deleteJob(queue);
	
	return 0;
}
