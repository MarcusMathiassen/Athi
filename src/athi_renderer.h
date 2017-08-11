#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include <functional>
#include <vector>

// Stores all draw calls made each frame
extern std::vector<std::function<void()> > command_buffer;

struct Renderer
{
  void commit()
  {
    // Execute all stores calls
    for (auto& c: command_buffer) c();

    // Clear the buffer for next frame
    command_buffer.clear();
  }
};

void render_call(const std::function<void()>& f);
