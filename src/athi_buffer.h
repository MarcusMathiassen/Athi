#pragma once

#include "athi_typedefs.h"

#include <unordered_map> // unordered_map

#define GLEW_STATIC
#include <GL/glew.h>

enum buffer_usage {
  STREAM_DRAW = GL_STREAM_DRAW,
  STREAM_READ = GL_STREAM_READ,
  STREAM_COPY = GL_STREAM_COPY,
  STATIC_DRAW = GL_STATIC_DRAW,
  STATIC_READ = GL_STATIC_READ,
  STATIC_COPY = GL_STATIC_COPY,
  DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
  DYNAMIC_READ = GL_DYNAMIC_READ,
  DYNAMIC_COPY = GL_DYNAMIC_COPY,
};

enum buffer_type {
  ARRAY_BUFFER = GL_ARRAY_BUFFER,
  ATOMIC_COUNTER_BUFFER = GL_ATOMIC_COUNTER_BUFFER,
  COPY_READ_BUFFER = GL_COPY_READ_BUFFER,
  COPY_WRITE_BUFFER = GL_COPY_WRITE_BUFFER,
  DISPATCH_INDIRECT_BUFFER = GL_DISPATCH_INDIRECT_BUFFER,
  DRAW_INDIRECT_BUFFER = GL_DRAW_INDIRECT_BUFFER,
  ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
  PIXEL_PACK_BUFFER = GL_PIXEL_PACK_BUFFER,
  PIXEL_UNPACK_BUFFER = GL_PIXEL_UNPACK_BUFFER,
  QUERY_BUFFER = GL_QUERY_BUFFER,
  SHADER_STORAGE_BUFFER = GL_SHADER_STORAGE_BUFFER,
  TEXTURE_BUFFER = GL_TEXTURE_BUFFER,
  TRANSFORM_FEEDBACK_BUFFER = GL_TRANSFORM_FEEDBACK_BUFFER,
  UNIFORM_BUFFER = GL_UNIFORM_BUFFER,
};

struct VBO {
  u32 handle;
  u32 attrib_num;
  size_t data_size;
  buffer_usage usage{STATIC_DRAW};
  buffer_type type{ARRAY_BUFFER};

  void bind() const noexcept { glBindBuffer(type, handle); }
};

class GPUBuffer {
 private:
  u32 vao;
  std::unordered_map<string, VBO> vbos;
  u32 attrib_counter{0};

 public:
  GPUBuffer() = default;
  ~GPUBuffer() {
    glDeleteVertexArrays(1, &vao);
    for (auto & [ name, vbo ] : vbos) glDeleteBuffers(1, &vbo.handle);
  }

  void init() noexcept {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
  }

  template <class T>
  void update(const string& name, T* data, size_t data_size) noexcept {
    // @Performance: this could be explicitly called in the future
    glBindVertexArray(vao);

    auto& vbo = vbos.at(name);
    glBindBuffer(vbo.type, vbo.handle);

    if (data_size > vbo.data_size) {
      glBufferData(vbo.type, data_size, &data[0], vbo.usage);
      vbo.data_size = data_size;
    } else {
      glBufferSubData(vbo.type, 0, vbo.data_size, &data[0]);
    }
  }

  template <class T>
  void add(const string& name, T* data, size_t data_size,
           buffer_type type = ARRAY_BUFFER, buffer_usage usage = STATIC_DRAW,
           u32 data_members = 1, u32 stride = 0, void* pointer = 0,
           u32 divisor = 0) noexcept {
    VBO vbo;
    vbo.attrib_num = attrib_counter;
    vbo.usage = usage;
    vbo.type = type;

    glGenBuffers(1, &vbo.handle);
    glBindBuffer(type, vbo.handle);
    glBufferData(type, data_size, &data[0], usage);

    switch (type) {
      case ELEMENT_ARRAY_BUFFER: { /* Do nothing */
      } break;

      default: {
        glEnableVertexAttribArray(attrib_counter);
        glVertexAttribPointer(attrib_counter, data_members, GL_FLOAT, GL_FALSE,
                              stride, (void*)pointer);

        if (divisor) glVertexAttribDivisor(attrib_counter, divisor);
      } break;
    }

    ++attrib_counter;

    vbos[name] = vbo;
  }

  void add_prototype(const string& name, u32 data_members,
                     buffer_type type = ARRAY_BUFFER, size_t stride = 0,
                     size_t pointer = 0, u32 divisor = 0) noexcept {
    VBO vbo;
    vbo.attrib_num = attrib_counter;
    vbo.type = type;

    glGenBuffers(1, &vbo.handle);
    glBindBuffer(type, vbo.handle);

    glEnableVertexAttribArray(attrib_counter);
    glVertexAttribPointer(attrib_counter, data_members, GL_FLOAT, GL_FALSE,
                          stride, (void*)(pointer));

    if (divisor) {
      glVertexAttribDivisor(attrib_counter, divisor);
    }

    ++attrib_counter;

    vbos[name] = vbo;
  }

  void add_prototype_mat(const string& name, u32 data_members,
                         buffer_type type = ARRAY_BUFFER, size_t stride = 0,
                         size_t pointer = 0, u32 divisor = 0) noexcept {
    VBO vbo;
    vbo.attrib_num = attrib_counter;
    vbo.type = type;

    glGenBuffers(1, &vbo.handle);
    glBindBuffer(type, vbo.handle);

    u32 attribs_made = attrib_counter;
    for (u32 i = 0; i < data_members; ++i) {
      glEnableVertexAttribArray(i + attrib_counter);
      glVertexAttribPointer(i + attrib_counter, data_members, GL_FLOAT,
                            GL_FALSE, stride, (void*)(i * pointer));

      if (divisor) {
        glVertexAttribDivisor(i + attrib_counter, divisor);
      }

      ++attribs_made;
    }

    attrib_counter += data_members;

    vbos[name] = vbo;
  }

  void bind() const noexcept { glBindVertexArray(vao); }
};
