#pragma once

#include "../athi_typedefs.h"

#include "athi_shader.h" // Shader
#include "athi_buffer.h" // Buffer
#include "athi_commandbuffer.h" // CommandBuffer

#include "../athi_settings.h"  // console
#include "../athi_utility.h"  // read_file

#include <functional>
#include <unordered_map>

extern vector<std::function<void()>> command_buffer;

void render();

void render_call(const std::function<void()>& f);

struct Renderer
{
  string name;

  Shader shader;
  Buffer buffer;

  Renderer() = default;
  Renderer(const string& pname);
  void bind() noexcept;
  void draw(const CommandBuffer& cmd) noexcept;
  Shader& make_shader();
  void update_buffer(const string& name, void* data, size_t data_size) noexcept;
  Vbo& make_buffer(const string& name) noexcept;
  void finish() noexcept;
};

static vector<Renderer> renderers;

static Renderer& make_renderer(const string& name) {
  return renderers.emplace_back(name);
}
