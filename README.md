# LemonDB: An In-memory Database with Multi-threading Query Written by C++

[![Build Status](https://focs.ji.sjtu.edu.cn:2222/api/badges/ve482-21/p2-group-06/status.svg?ref=refs/heads/multi_threads)](https://focs.ji.sjtu.edu.cn:2222/ve482-21/p2-group-06)

## Remarks

This project is a course project in *VE482 Operating System* [@UM-SJTU Joint Institute](https://www.ji.sjtu.edu.cn/). In general, we implement an in-memory database `LemonDB` using C++ featured multi-threading.

## Documentation

For this documentation, we focus on mainly 4 points:
  1. Architecture Design: Thread Pool
  2. Multi-threading Resolutions: Partition programming
  3. Performance Improvements: Complexity, `std::future` and Partition Strategy
  4. Future Improvement: Scheduler and Concurrency Query

### 1. Thread Pool
The thread pool is defined in the file `/src/multithreads/MultiThread.hpp`. Here are the members of the class Thread_Pool:
```cpp
class Thread_Pool {
private:
  std::mutex lockx;
  std::queue<void_Task> Task_assemble;
  std::vector<std::thread> pool_vector;
  std::condition_variable cv;
  std::atomic<bool> done;
  std::atomic<int> idleThreadNum;
  
  void addTask(void_Task task)
  void_Task getTask();
  void scheduler();
  
public:
  Thread_Pool();
  void pool_set(int temp);
  ~Thread_Pool();
  void start();
  void closed();
  int Task_size();
  int Thread_count();

  template <class F, class... Args>
  auto Submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>;
}
```
- The thread pool is used to manage all the worker threads. 
- The thread pool is thread-safe with the protection of the mutex `lockx` and the use of `std::atomic` variables.
- The `done` variable is used to indicate whether all job has been done.
- The `Task_assemble` queue stores all the tasks that the worker threads need to finish, details about how the queue is filled will be explained in the next section.
- Upon construction, the thread pool creates the threads and stores them in the `pool_vector`, where the number of threads is either `std::thread::hardware_concurrency()` or the user input.
- Once initialized, the worker threads start running the function `void scheduler()`, in which the worker thread loops to get the first task from the `Task_assemble` and executes the task, until all tasks are finished.

### 2. Partition Programming

We found that the tables usually feature very large sizes, and most of the queries, other than `LOAD, DUMP, COPYTABLE ... ` queries for table management, data queries `SELECT, SUM, MIN, ...` must traverse the table row by row. Consequentially, traversing data queries with single thread account for much time of execution. 

Based on this observation, we decide to divide the large table into several sub table section according to the number of available threads. All the threads is assigned with a task to execute the query on the sub table at the same time. In this way, the table could be traversed parallelly and save a lot of time.

A typically example of `count` is shown below. 

```cpp
subtable_num = (unsigned int)(table.size()) / total_thread;
vector<future<int>> futures((unsigned long)total_thread);
for (int i = 0; i < (int)total_thread; i++)
  futures[(unsigned long)i] = worker.Submit(Sub_Count, i);
```

- `subtable_num` stands for the size of each sub table.

- `worker.submit`is used to upload tasks for thread to execute.

- `Sub_Count`is a function that each thread will execute. Its input is `i`, representing the index of each sub table. It will do the count query on sub table and then return `subcounter`.

  ```c
  int Sub_Count(int id) {
    auto head = copy_table->begin() + (id * (int)subtable_num);
    auto tail = id == (int)total_thread - 1 ? copy_table->end()
                                            : head + (int)subtable_num;
    int subcounter = 0;
    if (result.second) {
      for (auto item = head; item != tail; item++) {
        if (copy_this->evalCondition(*item)) {
          subcounter++;
        }
      }
    }
    return subcounter;
  }
  ```

- In the main thread, it will wait until all the threads finish its work and then combine the result. Take `COUNT` query as example, the partional count result returned by each thread could be accessed by `get()` method in `std::future`, which would be added up in `counter` to get the final query result.
  
  ```cpp
  for (size_t i = 0; i < total_thread; i++)
    counter = counter + futures[i].get();
  ```

### 3. Performance Improvements

#### 3.1 Complexity

To optimize the runtime, no matter in single thread or multi-threading version, we must ensure the time complexity of executing a query is O(n), where n is the number of table's row. However, at first we unconsciously invoke an O(n^2) time complexity by using the `erase()` method in `std::vector`.

After searching cpp reference and online resources, we notice that the complexity of `std::vector::erase()` is linear for one arbitrary element in vector, both to the length of the range erased and to the number of elements between the end of the range and the end of the container. As a result, for `DELETE` query, the time complexity is O(n^2), which is an incredibility slow method when it comes to bigger table and more datum to delete.

At first, we came up with one method, which sacrifices place for time. We can simply keep moving the row which **don't** need to be deleted to a new table, traverse through the old one and finally swap the new table with the origin one. However, we found it still relatively slow comparing to the benchmark since it behaves worse in cases that few deletion is needed, as a lot of move and copy are needed.

Then, we came up with another method. Luckily, erasing an element from the end only takes constant time. Also, according to `p2.pdf`, "the records (rows) are unordered in a table", we come up another solution. With this two hints, we can simply copy the last element in table to the row to be delete, and delete the last row. In this case, only two steps are involved and it performs better in cases of big table with few deletions. Of course, tradeoffs always exist, but considering common using scenarios, we finalized this solution.

```cpp
data[index] = std::move(data.back());
data.pop_back();
```

#### 3.2 `std::future`

As we all know, when multi-threading, modify a common global variable is dangerous. To prevent race condition, we need to use mutex. However, the method `lock()` and `unlock()` is quite time-consuming. To prevent the lock as much as possible without invoking race condition, we use `std::future` in thread pool. Generally, it reserves position for functions returned in future when submitting multi-threading tasks. Then, we could get the result as long as the function submitted to worker threads returns, and add them up to get the final result.

```cpp
for (size_t i = 0; i < total_thread; i++)
  futures[i] = worker.Submit(Sub_Func, i);
```

Since the `get()` member function waits until the future has a valid result and retrieves it. It serves as the `join()` method in `pthread` to wait until all the threads have finished to return a final result, in this case the thread pool can achieve synchronization even if the task number exceeds the number of worker threads. And this process is only finished in the main thread where the tasks are submitted, as a result, it is guaranteed that no race condition could ever happen.

```cpp
for (size_t i = 0; i < total_thread; i++)
  counter = counter + futures[i].get();
```

#### 3.3 Partition Strategy

With the thread pool structure and partition programming, another factors that affect the run-time is the partition fraction. It is obvious that keeping a lot of thread alive is resources- and time-consuming, while a small fraction such as only dividing table into 2 parts may lead to fewer efficiency improvement. With rounds of testing, we found that the consumption in time of maintaining a new worker thread is approximately equals scanning over 2000 lines. That is to say, it's better to fill one thread with 2000 lines when doing multi-threading. As a result, we only wake up the number of threads which equals to the total table size divided by 2000. Of course, this is only a rough estimation, the cost and benefit vary as many factors such as CPU performance and memory read-write performance. But on the server, it's approximately the best parameters.

```cpp
total_thread = table.size() / 2000 + 1;
```

### 4. Future Improvement: Scheduler and Concurrency Query

In this project, we take advantage of the partition programming, which uses the idea of divide and conquer with the help of multi-threading. But generally, we execute the query one by one in a primative way. Enlightening by the modern CPU of its out-of-order execution, we could even make the execution of query parallelly.

To achieve this, we could design to determine how much query to execute together, fetch queries that operate on different table to prevent race condition and even schedule the execution order depending on different priority, estimated execution time or static/dynanmic method.

## Developer Quick Start

See INSTALL.md for instructions on building from source.

`ClangFormat` and `EditorConfig` are used to format codes.

Hint to using `ClangFormat`:
`find . -name "*.m" -o -name "*.h" | sed 's| |\\ |g' | xargs clang-format -i`

And make sure your code editor has `EditorConfig` support.

## Acknowledgement

We appreciate every help received from professor Manuel, TAs, authors of paper & online resources, teammates and fellows.
