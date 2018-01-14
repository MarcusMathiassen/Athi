#pragma once

#include <functional>
#include <vector>

extern std::vector<std::function<void()>> command_buffer;

void render();

void render_call(const std::function<void()> &f);

class Renderer {
private:
};
