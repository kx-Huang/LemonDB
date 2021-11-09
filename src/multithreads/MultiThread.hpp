
#ifndef MULTITHREAD_H
#define MULTITHREAD_H

#include "../db/Database.h"
#include "../query/QueryResult.h"
#include <string>
#include <pthread.h>
#include <condition_variable>
#include <memory>
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <future>
#include <atomic>
typedef std::function<void()> void_Task;
/****************************************************/
/****************************************************/
namespace Thread_Pool {
class Thread_Pool {
    #define DEFAULT_THREAD_NUM 4
    private:
    
    std::mutex lockx;
    std::queue <void_Task> Task_assemble;
    std::vector<std::thread> pool_vector;
    std::condition_variable cv;
    std::atomic<bool> done;    
	std::atomic<int> idleThreadNum;
  //  void worker_thread();
    
    void addTask(void_Task task){
	std::lock_guard<std::mutex> lock{ lockx };
	Task_assemble.push(task);
	cv.notify_one();
}
    void_Task getTask(){
	std::unique_lock<std::mutex> lock(lockx); 
	while (Task_assemble.empty() && !done) {
		cv.wait(lock);
	} 
	if (done) {
		return void_Task();
	}
	void_Task temp = std::move(Task_assemble.front());
	Task_assemble.pop();
	cv.notify_one();
	return temp;
}

    void scheduler(){
	while (!done.load()) {
		void_Task task(getTask());
		if (task) {
		idleThreadNum--;
		task();
		idleThreadNum++;
		}			
	}
}
    

    public:

    Thread_Pool(){
        idleThreadNum = 0;
	    done = false;
    }

	void pool_set(int temp){
	    for (int i = 0; i < temp; i++)
		{
			pool_vector.emplace_back(&Thread_Pool::scheduler, this);
		}
	}

    ~Thread_Pool(){
			closed();
			while (!Task_assemble.empty()) {
				Task_assemble.pop();
			}
			cv.notify_all();
			for (std::thread& thread : pool_vector) {
				if (thread.joinable()) {
					thread.join(); 
				}
			}
			pool_vector.clear();
}

    void start(){
  if (done == true) done.store(false);
		cv.notify_all();
}

    void closed(){
  if (!done) done.store(true);
}

    int Task_size(){
  //Return the left task.size
   return (int)Task_assemble.size(); 
}

    int Thread_count(){
  //Return the current idle_threads
  return idleThreadNum; 
}



    template<class F, class... Args>
		auto Submit(F&& f, Args&&... args)->std::future<decltype(f(args...))>{

			using RetType = decltype(f(args...)); 
			std::shared_ptr<std::packaged_task<RetType()>> task = std::make_shared<std::packaged_task<RetType()>>(
				std::bind(std::forward<F>(f), std::forward<Args>(args)...)
				);
			std::future<RetType> future = task->get_future();
			addTask([task](){
				(*task)(); 
			});

			return future;

        }
};
}

extern Thread_Pool::Thread_Pool worker;
/************************************************************/
// Previous Design, just leave here.
/*
void ThreadNum_Detection();

void set_ThreadNum(unsigned int temp);

unsigned int get_ThreadNum();
*/
#endif
