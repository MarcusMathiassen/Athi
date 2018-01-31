#pragma once

#include "../athi_typedefs.h"

#include <functional>

extern vector<std::function<void()>> command_buffer;

void render();

void render_call(const std::function<void()> &f);

class Renderer {
private:
};
