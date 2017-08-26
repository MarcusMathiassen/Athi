#pragma once

#include <functional>
#include <string>
#include <thread>
#include <vector>

class Task {
 public:
  std::string desc;  // Description of the task
  std::function<void()> task;

  Task(const std::string& desc, const std::function<void()>& task)
      : desc(desc), task(task) {}
  void run();
};

class TaskManager {
 private:
  // TaskManager will keep running as long as this is true
  bool is_running{true};

  // Threads
  std::vector<bool> available_threads;
  std::vector<std::thread> thread_pool;

  // Tasks
  std::atomic<int> task_count;
  std::vector<Task> task_queue;

 public:
  void start();
  void giveTask(const std::string& desc, const std::function<void()>& task);
  void printActiveWorkers();
  void printInfo();

  TaskManager();
  ~TaskManager();
};
