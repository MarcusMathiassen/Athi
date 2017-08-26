
#include "athi_task_manager.h"
#include <iostream>
#include <thread>

TaskManager::TaskManager() {
  const auto num_threads = std::thread::hardware_concurrency();
  thread_pool.reserve(num_threads);
}

TaskManager::~TaskManager() {
  for (auto& thread : thread_pool) {
    if (thread.joinable()) {
      thread.join();
      std::cout << "Thread " << thread.get_id() << ": joined." << '\n';
    } else {
      std::cout << "Thread " << thread.get_id() << ": not joinable." << '\n';
    }
  }
}
void TaskManager::start() {
  while (is_running) {
    for (auto& task : task_queue) {
      std::cout << "Task: " << task.desc << '\n';
    }
    is_running = false;
  }
}
void TaskManager::giveTask(const std::string& desc,
                           const std::function<void()>& task) {
  task_queue.emplace_back(desc, task);
}
