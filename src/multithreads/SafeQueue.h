#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H
#include <condition_variable>
#include <mutex>
#include <queue>
template <typename T> class SafeQueue {
private:
  // only one thread can access queue_data at a time
  mutable std::mutex mut;
  std::queue<T> queue_data;
  std::condition_variable queue_cond;

public:
  SafeQueue() {}

  void push(T val) {
    // tries to obtain the lock
    // ensures mutex never locked forever
    std::lock_guard<std::mutex> lock(mut);
    queue_data.push(val);
    // wake up exactly one blocked thread
    queue_cond.notify_one();
  }

  /**
   * @brief waits until queue is non-empty,
   *        then pops
   * @param value
   */
  void wait_and_pop(T &value) {
    // unique lock allows for the transfer
    // of ownership of lock
    std::unique_lock<std::mutex> lock(mut);
    queue_cond.wait(lock, [this] { return !queue_data.empty(); });
    value = std::move(queue_data.front());
    queue_data.pop();
  }

  /**
   * @brief tries to pop()
   *        returns true on succeed, moves poped value to value
   * @param value
   */
  bool try_pop(T &value) {
    std::lock_guard<std::mutex> lock(mut);
    if (queue_data.empty())
      return false;
    value = std::move(queue_data.front());
    queue_data.pop();
    return true;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mut);
    return queue_data.empty();
  }
};

#endif // PROJECT_SAFEQUEUE_H