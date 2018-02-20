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

#include "../athi_settings.h"
#include "../athi_typedefs.h"
#include "opengl_utility.h"

#include <unordered_map>  // unordered_map

#include <GL/glew.h>

enum buffer_usage {
  stream_draw = GL_STREAM_DRAW,
  stream_read = GL_STREAM_READ,
  stream_copy = GL_STREAM_COPY,
  static_draw = GL_STATIC_DRAW,
  static_read = GL_STATIC_READ,
  static_copy = GL_STATIC_COPY,
  dynamic_draw = GL_DYNAMIC_DRAW,
  dynamic_read = GL_DYNAMIC_READ,
  dynamic_copy = GL_DYNAMIC_COPY,
};

enum buffer_type {
  array_buffer = GL_ARRAY_BUFFER,
  atomic_counter = GL_ATOMIC_COUNTER_BUFFER,
  copy_read = GL_COPY_READ_BUFFER,
  copy_write = GL_COPY_WRITE_BUFFER,
  dispatch_indirect = GL_DISPATCH_INDIRECT_BUFFER,
  draw_indirect = GL_DRAW_INDIRECT_BUFFER,
  element_array = GL_ELEMENT_ARRAY_BUFFER,
  pixel_pack = GL_PIXEL_PACK_BUFFER,
  pixel_unpack = GL_PIXEL_UNPACK_BUFFER,
  query = GL_QUERY_BUFFER,
  shader_storage = GL_SHADER_STORAGE_BUFFER,
  texture = GL_TEXTURE_BUFFER,
  transform_feedback = GL_TRANSFORM_FEEDBACK_BUFFER,
  uniform = GL_UNIFORM_BUFFER,
};

struct Vbo {
  u32 handle{0};
  void* data{nullptr};
  size_t data_size{0};
  buffer_usage usage{buffer_usage::static_draw};
  buffer_type type{buffer_type::array_buffer};
  u32 data_members{1};
  GLsizei stride{0};
  size_t pointer{0};
  u32 divisor{0};
  bool is_matrix{false};
  u32 attrib_num{0};
};

struct Buffer {
  u32 vao;
  std::unordered_map<string, Vbo> vbos;
  u32 attrib_counter{0};

  Buffer() = default;
  ~Buffer();

  void update(const string& name, void* data, size_t data_size) noexcept {
    glBindVertexArray(vao);
    check_gl_error();

    // error checking
    if constexpr (DEBUG_MODE)
    {
        if (vbos.find(name) == vbos.end())
        {
          console->error("buffer: {} does not exist. Typo?", name);
          return;
        }
    }

    auto& vbo = vbos.at(name);
    glBindBuffer(vbo.type, vbo.handle);
    check_gl_error();
    if (data_size > vbo.data_size) {
      glBufferData(vbo.type, data_size, data, vbo.usage);
      check_gl_error();
      vbo.data_size = data_size;
    } else {
      glBufferSubData(vbo.type, 0, vbo.data_size, data);
      check_gl_error();
    }
  }

  template <class T>
  void update(const string& name, vector<T>& data) noexcept {
    glBindVertexArray(vao);
    check_gl_error();

    // error checking
    if constexpr (DEBUG_MODE)
    {
        if (vbos.find(name) == vbos.end())
        {
          console->error("buffer: {} does not exist. Typo?", name);
          return;
        }
    }

    const size_t data_size = data.size() * sizeof(data[0]);

    auto& vbo = vbos.at(name);
    glBindBuffer(vbo.type, vbo.handle);
    check_gl_error();
    if (data_size > vbo.data_size) {
      glBufferData(vbo.type, data_size, &data[0], vbo.usage);
      check_gl_error();
      vbo.data_size = data_size;
    } else {
      glBufferSubData(vbo.type, 0, vbo.data_size, &data[0]);
      check_gl_error();
    }
  }

  void bind() const noexcept;
  void finish() noexcept;
};
