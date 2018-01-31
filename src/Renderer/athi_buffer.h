#pragma once

#include "../athi_typedefs.h"
#include "../athi_settings.h"

#include <unordered_map> // unordered_map

#define GLEW_STATIC
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
  array = GL_ARRAY_BUFFER,
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
  u32 handle;
  string name;

  void* data{nullptr};
  size_t data_size{0};

  buffer_usage usage{buffer_usage::static_draw};
  buffer_type type{buffer_type::array};

  u32 data_members{0};

  size_t stride{0};
  size_t pointer{0};

  u32 divisor{0};

  bool is_matrix{false};
};

struct Buffer {

  u32 vao;
  std::unordered_map<string, Vbo> vbos;

  ~Buffer() {
    glDeleteVertexArrays(1, &vao);
    for (auto & [ name, vbo ] : vbos) glDeleteBuffers(1, &vbo.handle);
  }
  
  void update(const string& name, void* data, size_t data_size) noexcept {
    auto &vbo = vbos.at(name);
    glBindBuffer(vbo.type, vbo.handle);
    if (data_size > vbo.data_size) {
      glBufferData(vbo.type, data_size, data, vbo.usage);
      vbo.data_size = data_size;
    } else {
      glBufferSubData(vbo.type, 0, vbo.data_size, data);
    }
  }

  void bind() const noexcept {
    glBindVertexArray(vao);
  }

  void finish() noexcept {

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    u32 attrib_counter = 0;
    for (auto& [name, vbo] : vbos) {

      const auto data_size = vbo.data_size;
      const auto data_members = vbo.data_members;
      const auto type = vbo.type;
      const auto usage = vbo.usage;
      const auto stride = vbo.stride;
      const auto pointer = vbo.pointer;
      const auto divisor = vbo.divisor;
      const auto is_matrix = vbo.is_matrix;

      console->info("name: {}, data_members: {}, data_size: {}, is_matrix: {}", name, data_members, data_size, is_matrix);

      glGenBuffers(1, &vbo.handle);
      glBindBuffer(type, vbo.handle);

      if (vbo.data != nullptr) {
        glBufferData(type, data_size, vbo.data, usage);
      }

      u32 attribs_made = 0;
      if (is_matrix) {
        for (u32 i = 0; i < data_members; ++i) {
          glEnableVertexAttribArray(i + attrib_counter);
          glVertexAttribPointer(i + attrib_counter, data_members, GL_FLOAT,
                                GL_FALSE, stride, (void*)(i * pointer));

          if (divisor) {
            glVertexAttribDivisor(i + attrib_counter, divisor);
          }
        }
        attrib_counter += data_members;
      } else {
        glEnableVertexAttribArray(attrib_counter);
        glVertexAttribPointer(attrib_counter, data_members, GL_FLOAT, GL_FALSE, stride, (void*)pointer);
        if (divisor) glVertexAttribDivisor(attrib_counter, divisor);
        ++attrib_counter;
      }
    }
  }
};