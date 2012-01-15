/*
 * This class implements the Maleable Scheduler.
 */

#ifndef MALEABLE_SCHEDULER_H
#define MALEABLE_SCHEDULER_H

#include <vector>
#include <list>
#include <algorithm>
#include <boost/thread.hpp>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include "workqueue.h"

#define SIGBLOCKCORE SIGRTMIN+1
#define SIGUNBLOCKCORE SIGRTMIN+2
#define SIGSTOPSIGNALTHREAD SIGRTMIN+3


namespace Scheduler {

class MaleableScheduler {
	private:
		// the current jobs
		std::list<WorkQueue*> jobs;
		
		// the current "hard" schedule, this is the schedule for the pinned threads
		// if a core is disabled, the "hard" schedule is NULL for that thread
		std::vector<WorkQueue*> schedule;
		
		// each thread needs a condition variable and a mutex
		std::vector<boost::thread*> threads;
		std::vector<boost::condition_variable*> thread_cd;
		std::vector<boost::mutex*> thread_mutex;
		
		// the number of (hardware) threads, this is the number of threads that are
		// started and scheduled
		int p;
		
		// the number of sleeping threads
		int sleeping;
		
		// mutex and conditionvariable for manageing sleeping threads
		boost::mutex sleeping_mutex;
		boost::condition_variable sleeping_cd;
		
		// set to true when this object is being destructed, and all threads
		// need to return
		volatile bool destruct;
		
		// thread for signal handling, the only thread not blocking signals
		boost::thread* signalThread;
		// for signals and the received info about blocked/available cores
		volatile bool receivedSignal;
		// true if the object is to be destructed and the signal thread has to return
		volatile bool receivedStopSignal;
		// the signal handler writes into this variable
		volatile bool * new_availableCores; 
		// this is also used by the scheduler, and the p threads to decide wether 
		// they are working on their schedule or going to sleep (when their core gets disabled)
		volatile bool * availableCores; 
		
		// Singleton: this holds the only Reference to the only object of this class
		static MaleableScheduler * instance;
		
		void pin_to_core(int cpuid) {
			cpu_set_t set;
			CPU_ZERO(&set);
			CPU_SET(cpuid,&set);
			sched_setaffinity(0,sizeof(cpu_set_t),&set);
		}
		
		void block_all_signals() {
			sigset_t sigset;
			sigfillset(&sigset);
			pthread_sigmask(SIG_BLOCK, &sigset, NULL);
		}

		
		struct ThreadWork {
			void operator()(MaleableScheduler* scheduler, int coreid) {
				scheduler->pin_to_core(coreid);
				scheduler->block_all_signals();
				while(true) {
					boost::unique_lock<boost::mutex> lock(*(scheduler->thread_mutex[coreid]),boost::defer_lock_t());
					lock.lock();
					// wait until this thread is scheduled to a job
					while (scheduler->schedule[coreid] == NULL || scheduler->availableCores[coreid] == false) {
						boost::unique_lock<boost::mutex> l(scheduler->sleeping_mutex);
						scheduler->sleeping++;
						if (scheduler->sleeping == scheduler->p) scheduler->sleeping_cd.notify_all();
						l.unlock();
						scheduler->thread_cd[coreid]->wait(lock);
						l.lock();
						scheduler->sleeping--;
						if (scheduler->destruct) return;
					}
					
					scheduler->pin_to_core(coreid);
					scheduler->block_all_signals();
					
					// do one paket of work
					WorkQueue* queue = scheduler->schedule[coreid];
					lock.unlock();
					
					queue->wait_and_workOne();

					// TODO maybe reschedule
				}
			}
		} threadWork;
		

		
		void scheduleOnCore(WorkQueue* job, int core) {
			boost::unique_lock<boost::mutex> lock(*thread_mutex[core]);
			bool notify = schedule[core] == NULL;
			schedule[core] = job;
			if (notify) {
				thread_cd[core]->notify_all();
			}
		}
		
		/*
		 * Signal Handler for the Realtime Linux Signals. This receives the information
		 * when cores are blocked and unblocked.
		 */
		static void sighandler(int signum, siginfo_t * info, void * context) {
			MaleableScheduler* sched = MaleableScheduler::singleton();
			if (signum == SIGSTOPSIGNALTHREAD) {
				sched->receivedStopSignal = true;
			} else if (info->si_code == SI_QUEUE) {
				int core = info->si_value.sival_int;
				if (core < sched->p) {
					if (signum == SIGBLOCKCORE) {
						sched->new_availableCores[core] = false;
						sched->receivedSignal = true;
					} else if (signum == SIGUNBLOCKCORE) {
						sched->new_availableCores[core] = true;
						sched->receivedSignal = true;
					}
				}	
			}
		}
		
		/*
		 * Function for the thread, receiving the signals.
		 */
		static void signalthreadfunction(MaleableScheduler* sched) {
			// prepare signal handler
			struct sigaction act;
			sigset_t blockAllMask;
			sigfillset(&blockAllMask);
			act.sa_sigaction = &MaleableScheduler::sighandler;
			act.sa_mask = blockAllMask;
			act.sa_flags = 0 | SA_SIGINFO;
			
			// block all signals
			pthread_sigmask(SIG_SETMASK,&blockAllMask,NULL);
			
			
			// set signal handler
			sigaction(SIGBLOCKCORE, &act, NULL);
			sigaction(SIGUNBLOCKCORE, &act, NULL);
			sigaction(SIGSTOPSIGNALTHREAD, &act, NULL);
			
			
			// create signal mask, that blocks all signals but the
			// ones that we want to receive
			sigset_t blockAllButMask;
			sigfillset(&blockAllButMask);
			sigdelset(&blockAllButMask, SIGBLOCKCORE);
			sigdelset(&blockAllButMask, SIGUNBLOCKCORE);
			sigdelset(&blockAllButMask, SIGSTOPSIGNALTHREAD);

			
			// wait for signals and handle them
			while (true) {
				sigsuspend(&blockAllButMask);

				// return if Scheduler object is destructed
				if(sched->receivedStopSignal) return;
				
				// enable/disable cores when this signal arrives
				if (sched->receivedSignal) {
					sched->receivedSignal = false;
					
					// set new schedule
					for (int i = 0; i < sched->p; i++) {
						if (sched->availableCores[i] == false && sched->new_availableCores[i] == true) {
							// core i is unblocked:
							// the lock is necessary here, otherwise (scenario):
							// -> Pinned Thread checks available Cores, it is not available
							// -> Pinned Thread prepares to go to sleep
							// -> Signal hander thread (here) sets availableCores[i] to true and
							//    notifies the (not yet sleeping thread)
							// -> the notify does not reach the pinned thread, because it is not
							//    yet sleeping
							// -> the pinned thread goes to sleep, the notify never reaches it, so
							//    it does not wake up (i.e. might sleep forever)
							boost::unique_lock<boost::mutex> lock(*(sched->thread_mutex[i]));
							sched->availableCores[i] = true;
							// notify thread that it can continue work
							sched->thread_cd[i]->notify_all();
						} else if (sched->availableCores[i] == true && sched->new_availableCores[i] == false) {
							// the core i is blocked
							sched->availableCores[i] = false;
						}
					}
				}
			}
		}
		
		
		void init(int num_threads) {
			// block signals for signal thread in main thread
			sigset_t mask;
			sigemptyset(&mask);
			sigaddset(&mask, SIGBLOCKCORE);
			sigaddset(&mask, SIGUNBLOCKCORE);
			sigaddset(&mask, SIGSTOPSIGNALTHREAD);
			pthread_sigmask(SIG_BLOCK,&mask,NULL);
		
		
			p = num_threads;
			
			// init variables for signal handling/core blocking
			receivedSignal = false;
			receivedStopSignal = false;
			new_availableCores = new volatile bool[p];
			availableCores = new volatile bool[p];
			for (int i = 0; i < p; i++) {
				availableCores[i] = true;
				new_availableCores[i] = true;
			}
			// start signal thread
			signalThread = new boost::thread(&MaleableScheduler::signalthreadfunction,this);
			
			// init variables
			destruct = false;
			sleeping = 0;
			threads = std::vector<boost::thread*>(p,NULL);
			schedule = std::vector<WorkQueue*>(p,NULL);
			thread_cd = std::vector<boost::condition_variable*>(p,NULL);
			thread_mutex = std::vector<boost::mutex*>(p,NULL);
			// init and start work-threads
			for (int i = 0; i < p; i++) {
				thread_mutex[i] = new boost::mutex();
				thread_cd[i] = new boost::condition_variable();
				threads[i] = new boost::thread(threadWork,this,i);
			}
			
			// wait until all threads are ready
			{
				boost::unique_lock<boost::mutex> l(sleeping_mutex);
				while (sleeping < p) {
					sleeping_cd.wait(l);
				}
			}
		}
		
		/* 
		 * Creates the Scheduler with as many Threads as there are
		 * Hardware-Threads in the System.
		 */
		MaleableScheduler() {
			int hwthreads = boost::thread::hardware_concurrency();
			init(hwthreads);
		}
		
		~MaleableScheduler() {
			// quit signal thread
			pthread_kill(signalThread->native_handle(), SIGSTOPSIGNALTHREAD);
			signalThread->join();
			//std::cout << "Signal-Thread joined " << std::endl;
		
			// clear schedule
			for (std::vector<WorkQueue*>::iterator it = schedule.begin(); it != schedule.end(); ++it) {
				*it = NULL;
			}
			
			// release waiting Threads from Job Queues
			for (std::list<WorkQueue*>::iterator i = jobs.begin(); i != jobs.end(); ++i) {
				(*i)->releaseWaitingThreads();
			}
			//std::cout << "released queue threads" << std::endl;
			// wait for all threads to return here
			{
				//std::cout << "trying lock" << std::endl;
				boost::unique_lock<boost::mutex> l(sleeping_mutex);
				//std::cout << "lock" << std::endl;
				//std::cout << "sleeping: " << sleeping << "/" << p <<  std::endl;
				while (sleeping < p) {
					sleeping_cd.wait(l);
				}
			}
			//std::cout << "threads returned" << std::endl;
			
			// finish last jobs and delete job objects
			for (std::list<WorkQueue*>::iterator i = jobs.begin(); i != jobs.end(); ++i) {
				delete (*i);
			}
			//std::cout << "deleted jobs" << std::endl;

			
			// quit threads
			for (int i = 0; i < p; i++) {
				thread_mutex[i]->lock();
			}
			destruct = true;
			for (int i = 0; i < p; i++) {
				thread_mutex[i]->unlock();
				thread_cd[i]->notify_all();
				threads[i]->join();
			}
			//std::cout << "got all threads" << std::endl;
		
			// delete objects
			for (int i = 0; i < p; i++) {
				delete thread_cd[i];
				delete thread_mutex[i];
				delete threads[i];
			}
		
		}
		
	public:		
		/*
		 * This implements the Singleton Pattern, and returns the only Reference 
		 * to the MaleableScheduler
		 */
		static MaleableScheduler* singleton() {
			if (instance == NULL) {
				instance = new MaleableScheduler();
			}
			return instance;
		}
		
		/*
		 * Deletes the scheduler object and clears the instance to NULL, another call
		 * to singleton() creates a new object.
		 */
		static void deleteSingleton() {
			if (instance != NULL) {
				delete instance;
				instance = NULL;
			}
		}
	
		/* 
		 * Creates a new Queue and adds it as new Job to the Job list. Then returns the 
		 * new WorkQueue for the procedure generating Workpakets.
		 */
		WorkQueue* newJob() {
			WorkQueue* newjob = new WorkQueue();
			jobs.push_back(newjob);
			// TODO: schedule?
			return newjob;
		}
		
		/*
		 * Tells the Scheduler, that the Job represented by the given Queue is done,
		 * and the Queue can be destructed, and the Threads working on that Queue be
		 * rescheduled.
		 */
		 // TODO this function is not yet safe
		void deleteJob(WorkQueue* jobQueue) {
			// delete job from list
			std::list<WorkQueue*>::iterator pos = std::find(jobs.begin(),jobs.end(),jobQueue);
			if (pos != jobs.end()) {
				jobs.erase(pos);
			}
			// set the schedule for all threads that worked on that job to NULL
			for (std::vector<WorkQueue*>::iterator it = schedule.begin(); it != schedule.end(); ++it) {
				if (*it == jobQueue) {
					*it = NULL;
				}
			}
			// destruct the Queue, releasing all threads that are still waiting on that Queue.
			// TODO destructing queue before all threads are sleeping in Scheduler is potentially
			// 		dangerous.
			std::cout << "Attention: using deleteJob() is not safe!" << std::endl;
			delete jobQueue;
			jobQueue = NULL;
			
		}
		
		
		/*
		 * Schedules the given Job onto the cores flagged in the given Bit-Vector
		 */
		void scheduleJob(WorkQueue* job, std::vector<bool>&cores) {
			
			for (int i=0; i < p; i++) {
				if (cores[i]) {
					scheduleOnCore(job,i);
				} else {
					scheduleOnCore(NULL,i);
				}
			}
		}
		
		/*
		 * Schedules the given Job onto the given Cores, and removes it from the others.
		 */
		void scheduleJob(WorkQueue* job, std::vector<int>&cores) {
			// convert to bitvector
			std::vector<bool> bitvec(p,false);
			for (std::vector<int>::iterator it = cores.begin(); it != cores.end(); ++it) {
				bitvec[*it] = true;
			}
			scheduleJob(job, bitvec);
		}
		
		void scheduleToFirst(WorkQueue* job, int cores) {
			for (int i = 0; i < p; i++) {
				if (i < cores) {
					scheduleOnCore(job,i);
				} else {
					scheduleOnCore(NULL,i);
				}
			}
		}
		
		/*
		 * Schedules the given Job onto ALL Cores
		 */
		 void scheduleToAll(WorkQueue * job) {
		 	for (int i = 0; i < p; i++) {
		 		scheduleOnCore(job,i);
		 	}
		 }
};


// Initialize Singleton Variable
MaleableScheduler* MaleableScheduler::instance = NULL;


} // namespace
#endif
