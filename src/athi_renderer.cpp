#include "athi_renderer.h"

std::vector<std::function<void()> > command_buffer;

void render_call(const std::function<void()>& f)
{
  command_buffer.emplace_back(std::move(f));
}
