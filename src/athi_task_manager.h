#pragma once

#include <future>
#include <thread>
#include <functional>
#include <vector>
#include <memory>
#include "athi_typedefs.h"

struct Task;

std::atomic_bool run_threads{false};
std::vector<std::unique_ptr<std::thread> > thread_pool;
std::vector<Task*> task_buffer;

struct Task
{
  u32   task_id;
  void *function;
};

namespace Task_Manager
{
  void initialize();
  void shutdown();
  void process();

  void add_task(Task *task);
}
