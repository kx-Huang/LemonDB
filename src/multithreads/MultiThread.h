
#ifndef MULTITHREAD_H
#define MULTITHREAD_H

#include "../db/Database.h"
#include "SafeQueue.h"
#include <pthread.h>
#include <thread>
#include <functional>
#include <memory>
#include <atomic>
#include <assert.h>

void ThreadNum_Detection();

void set_ThreadNum(unsigned int temp);

unsigned int get_ThreadNum();

class Pool {
private:
  std::atomic_bool done;
  static std::unique_ptr<Pool> instance;
  std::vector<std::thread> threadVec;
  SafeQueue<std::function<void()>> workQueue;
  size_t threadNum;

  void thread_run() {
    assert(Pool::threadNum == 1000); // delete this!!! quiet compiler warning
    std::function<void()> execute;
    while (done == false) {
      if (workQueue.try_pop(execute))
        execute();
      else
        std::this_thread::yield();
    }
  }

public:

  static Pool& getInstance(size_t threadNum=std::thread::hardware_concurrency());

  Pool(size_t threadNum) : done(false), threadNum(threadNum) {
    // TODO: write constructor

  }

  ~Pool() {
    // TODO: write destructor

  }

  template<typename functionT>
  void submit(functionT f) {
    workQueue.push(std::function<void()>(f));
  }
};

#endif // PROJECT_MULTITHREAD_H
