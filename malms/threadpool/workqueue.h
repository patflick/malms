/*
 * This class implements a Blocking-Synchronized Queue with the Method blockuntildone()
 */

#ifndef WORKQUEUE_H
#define WORKQUEUE_H

#include <vector>
#include <deque>
#include <boost/thread.hpp>

namespace Scheduler {

class WorkQueueItem {
	public:
		virtual void operator()() = 0;
};

class WorkQueue {
	private:
		boost::mutex mut;
		boost::condition_variable cd;
		boost::condition_variable external_cd;
		std::deque<WorkQueueItem*> q;
		int active;
		int sleeping;
		volatile bool destruct;
		
	public:
	
		/*
		 * Constructor for the WorkQueue, initializing its attributes.
		 */
		WorkQueue() : active(0), sleeping(0), destruct(false) {
		}
		
		/*
		 * Destructor for the WorkQueue, this releases all the Threads that are still waiting
		 * in the Queue.
		 */
		~WorkQueue() {
			// release waiting threads, before objects get destroyed
			if (!destruct) {
				releaseWaitingThreads();
				// shouldnt happen
				std::cout << "WAAAAA This shoudl not happen, ARGH -.-" << std::endl;
			}
			//std::cout << "Destructor done!" << std::endl;
		}
		
		/*
		 * Gets the front Job from the Queue and completes that Job before returning.
		 * If the Queue is currently emtpy, this method blocks, until a job is available
		 * in the Queue or until the WorkQueue object is destructed, then the waiting Threads
		 * will be released. Threadsafe!
		 */
		void wait_and_workOne() {
			if (destruct) return;
			boost::unique_lock<boost::mutex> lock(mut);
			active++;
			// wait until queue is not empty and lock is aquired
			while (q.empty()) {
				active--;
				if (active == 0) {
					external_cd.notify_all();
				}
				sleeping++;
				//std::cout << "Thread sleeping at WorkQueue with active= " << active << " and sleeping= " << sleeping << std::endl;
				cd.wait(lock);
				sleeping--;
				active++;
				if (destruct) {
					active--;
					if (sleeping == 0 && active == 0) {
						// this thread is the last one to leave
						external_cd.notify_all();
					}
					//std::cout << "Thread Leaving WorkQueue with active= " << active << " and sleeping= " << sleeping << std::endl;
					return;
				}
			}
			
			WorkQueueItem* job = q.front();
			q.pop_front();
			// unlock before doing job
			lock.unlock();
			(*job)();
			lock.lock();
			active--;
			if (destruct && sleeping == 0 && active == 0)
				external_cd.notify_all();
			lock.unlock();
		}
		
		/*
		 * Pushes a new Job into the WorkQueue, this is Threadsafe!
		 */
		void push(WorkQueueItem* it) {
			boost::unique_lock<boost::mutex> lock(mut);
			q.push_back(it);
			cd.notify_one();
		}
		
		/*
		 * This Method blocks until there is no more active Thread working on
		 * the Jobs in the Queue.
		 */
		void blockuntildone() {
			boost::unique_lock<boost::mutex> l(mut);
			while (!q.empty() || active > 0) {
				external_cd.wait(l);
			}
		}
		
		/*
		 * Releases all waiting Threads and puts the Queue in a status where it does not accept new Threads.
		 */
		void releaseWaitingThreads() {
			boost::unique_lock<boost::mutex> lock(mut);
			destruct = true;
			//std::cout << "Notifying Sleeping Threads" << std::endl;
			cd.notify_all();
			// wait for all threads to leave before destructing attributes (mutex, etc)
			while (active != 0 || sleeping != 0) {
				external_cd.wait(lock);
				//std::cout << "Got Notified with active= " << active << " and sleeping= " << sleeping << std::endl;
			}
		}
};

} // namespace

#endif
