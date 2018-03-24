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

void render();
void render_clear();

void render_call(std::function<void()>&& f) noexcept;

struct render_desc
{
  u32 vao{0};
  u32 count{0};
  u32 num_indices{0};
  bool has_indices{false};
};

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

  template <class T>
  void update_buffer(const string& name, vector<T>& data) noexcept
  {
    buffer.bind();
    buffer.update(name, data);
  }

  void update_buffer(const string& name, void* data, size_t data_size) noexcept
  {
    buffer.bind();
    buffer.update(name, data, data_size);
  }

  Vbo& make_buffer(const string& name) noexcept;

  void finish() noexcept;
};


extern vector<Renderer> renderers;
Renderer& make_renderer(const string& name);
