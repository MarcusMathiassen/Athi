// Copyright (c) 2018 Marcus Mathiassen

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#pragma once

#include "athi_typedefs.h"
#include "./Utility/athi_constant_globals.h" // os
#include "./athi_utility.h" // get_begin_and_end

#include <thread> // thread
#include <cassert> // assert
#include <condition_variable> // condition_variable
#include <mutex> // mutex
#include <functional> // std::function<void()>
#include <future> // std::future
#include <queue> // std::queue

#ifdef __APPLE__
  #include <dispatch/dispatch.h>  // dispatch_apply
#endif

class Dispatch {
 private:
  s32 num_workers;
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop{false};

 public:
  std::queue<std::function<void()>> tasks;
  vector<std::thread> workers;
  s32 size() { return num_workers; }
  bool stopped() { return stop; }
  Dispatch(s32 num_workers = std::thread::hardware_concurrency() * 2) {
    assert(num_workers != 0 && "0 workers doesn't make sense.");
    workers.resize(num_workers);
    this->num_workers = num_workers;
    for (auto&& worker : workers) {
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
  ~Dispatch() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }
    condition.notify_all();
    for (auto&& worker : workers) worker.join();
  }

  template <class F, class... Args>
  auto enqueue(F&& f, Args&&... args) -> std::future<std::result_of_t<F(Args...)>> {
    using return_type = std::result_of_t<F(Args...)>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto result = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");
      tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return result;
  }

  template <class Container, class F>
  void parallel_for_each(Container& container, F&& f)
  {
    // Precalculate beginnings and ends
    vector<std::tuple<size_t,size_t>> begin_ends(num_workers);

    for (s32 i = 0; i < num_workers; ++i)
      begin_ends[i] = get_begin_and_end(i, container.size(), num_workers);

    if constexpr (os == OS::Apple)
    {
        // Using Apple's GCD
#ifdef __APPLE__
        dispatch_apply(num_workers, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0), ^(size_t i) {
            const auto [begin, end] = begin_ends[i];
            f(begin, end);
        });
#endif

    } else {

        // Use the naive thread pool
        vector<std::future<void>> results(num_workers);

        for (s32 i = 0; i < num_workers; ++i)
        {
            const auto [begin, end] = begin_ends[i];
            results[i] = enqueue(std::forward<F>(f), begin, end);
        }

        for (auto &&res : results)
          res.get();
    }
  }
};

extern Dispatch dispatch;
