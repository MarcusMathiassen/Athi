#include "athi_task_manager.h"

void Task_Manager::add_task(Task *task)
{
  task_buffer.emplace_back(task);
}

void Task_Manager::initialize()
{
  auto num_threads = std::thread::hardware_concurrency();
  thread_pool.reserve(num_threads);
}
