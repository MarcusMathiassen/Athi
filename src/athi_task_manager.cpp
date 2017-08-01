#include "athi_task_manager.h"

static void run_task();

void Task_Manager::add_task(Task *task)
{
  task_buffer.emplace_back(task);
}

void Task_Manager::initialize()
{
  auto num_threads = std::thread::hardware_concurrency();
  thread_pool.reserve(num_threads);
}

static void run_task()
{
  // auto id = std::this_thread::get_id();

  // while (run_threads)
  // {
  //   Task *task = task_buffer.pop_back();
  // }
}
