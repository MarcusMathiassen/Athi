// Copyright (c) 2018 Marcus Mathiassen

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

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
  buffer.vbos[name].attrib_num = buffer.attrib_counter++;
  return buffer.vbos[name]; 
}

void Renderer::finish() noexcept {
  shader.finish();
  buffer.finish();
}