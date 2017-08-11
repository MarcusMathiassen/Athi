#include "athi_renderer.h"
#include <mutex>

std::mutex render_mutex;
std::vector<std::function<void()> > command_buffer;

void render_call(const std::function<void()>& f)
{
  std::lock_guard<std::mutex> lock(render_mutex);
  command_buffer.emplace_back(std::move(f));
}

void render()
{
  std::lock_guard<std::mutex> lock(render_mutex);
  
  // Execute all stores callseq
  for (auto& c: command_buffer) c();
  
  // Clear the buffer for next frame
  command_buffer.clear();
}
