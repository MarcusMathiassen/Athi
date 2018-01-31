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
  u32 handle{0};
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

  Buffer() = default;
  ~Buffer();
  void update(const string& name, void* data, size_t data_size) noexcept;
  void bind() const noexcept;
  void finish() noexcept;
};