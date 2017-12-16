#pragma once

#include <iostream>
#include <functional>
#include <future>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <queue>
#include <cassert>

class Dispatch {
private:
  std::mutex                        queue_mutex;
  std::condition_variable           condition;
  bool                              stop{false};
public:
  std::queue<std::function<void()>> tasks;
  std::vector<std::thread>          workers;
  Dispatch(int num_workers = std::thread::hardware_concurrency())
  {
    assert(num_workers != 0 && "0 workers doesn't make sense.");
    workers.resize(num_workers);
    for (auto&& worker: workers) {
      worker = std::thread([this] {
        while (true) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) return;
            task = std::move(tasks.front());
            tasks.pop();
          }
          task();
        }
      });
    }
  }
  ~Dispatch()
  {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }
    condition.notify_all();
    for (auto&& worker: workers)
      worker.join();
  }

  template <class F, class ... Args>
  auto enqueue(F&& f, Args&& ... args) -> std::future<std::result_of_t<F(Args...)>>
  {
    using return_type = std::result_of_t<F(Args...)>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto result = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
      tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return result;
  }
};
