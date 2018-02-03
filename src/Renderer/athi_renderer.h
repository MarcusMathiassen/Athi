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
void render_clear();
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
