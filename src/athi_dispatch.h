#pragma once

#include <cassert>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

template <class T, class Container = std::vector<T>>
class queue {
private:
    typedef Container                                container_type;
    typedef typename container_type::value_type      value_type;
    typedef typename container_type::reference       reference;
    typedef typename container_type::const_reference const_reference;
    typedef typename container_type::size_type       size_type;
    
    Container buffer;
    size_type index_offset{0};
    
public:
    template <class ... Args>
    reference emplace(Args&&... args) noexcept { return buffer.emplace_back(std::forward<Args>(args)...);}
    void pop() noexcept {
        buffer[index_offset].~value_type();
        index_offset++;
    }
    T operator[](int index) const noexcept {
        return buffer[index_offset + index];
    }
    reference front() noexcept {
        return buffer[index_offset];
    }
    const_reference front() const noexcept {
        return buffer[index_offset];
    }
    bool empty() const noexcept { return (buffer.size() == index_offset); }
    void clear() noexcept { buffer.clear(); }
    size_type size() const noexcept { return buffer.size() - index_offset; }
    auto begin() noexcept { return buffer.begin() + index_offset; }
    auto end() noexcept { return buffer.end(); }
};


class Dispatch {
 private:
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop{false};

 public:
  std::queue<std::function<void()>> tasks;
  std::vector<std::thread> workers;
  std::size_t size() { return workers.size(); }
  bool stopped() { return stop; }
  Dispatch(int num_workers = std::thread::hardware_concurrency()) {
    assert(num_workers != 0 && "0 workers doesn't make sense.");
    workers.resize(num_workers);
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
  auto enqueue(F&& f, Args&&... args)
      -> std::future<std::result_of_t<F(Args...)>> {
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
};
