#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include <functional>
#include <vector>

// Stores all draw calls made each frame
extern std::vector<std::function<void()> > command_buffer;

void render();

void render_call(const std::function<void()>& f);
