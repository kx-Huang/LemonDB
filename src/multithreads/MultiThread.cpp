#include "MultiThread.h"

static unsigned int thread_num;

void ThreadNum_Detection() {
  thread_num = std::thread::hardware_concurrency();
  return;
}

unsigned int get_ThreadNum() { return thread_num; }

void set_ThreadNum(unsigned int temp) { thread_num = temp; }

std::unique_ptr<Pool> Pool::instance = nullptr;

Pool& Pool::getInstance(size_t threadNum) {
  if (Pool::instance == NULL) {
    Pool::instance = std::make_unique<Pool>(threadNum);
  }
  return *instance;
}
