#include "athi_renderer.h"

#include "../athi_utility.h" // profile
#include "../athi_settings.h" // console
#include <mutex>

std::mutex render_mutex;
vector<std::function<void()>> command_buffer;

void render_call(const std::function<void()> &f) {
  std::lock_guard<std::mutex> lock(render_mutex);
  command_buffer.emplace_back(std::move(f));
}

void render() {
  if (command_buffer.empty())
    return;
  profile p("render");
  std::lock_guard<std::mutex> lock(render_mutex);

  // Execute all stores callseq
  for (auto &c : command_buffer)
    c();

  // Clear the buffer for next frame
  command_buffer.clear();
}

Renderer::Renderer(const string& pname) : name(pname) {}
void Renderer::bind() noexcept {
  buffer.bind();
  shader.bind();
}

void Renderer::draw(const CommandBuffer& cmd) noexcept {
  buffer.bind();
  shader.bind();
  cmd.commit();
}

Shader& Renderer::make_shader() {
  return shader;
}

void Renderer::update_buffer(const string& name, void* data, size_t data_size) noexcept {
  buffer.bind();
  buffer.update(name, data, data_size);
}

Vbo& Renderer::make_buffer(const string& name) noexcept {
  return buffer.vbos[name]; 
}

void Renderer::finish() noexcept {
  shader.finish();
  buffer.finish();
}