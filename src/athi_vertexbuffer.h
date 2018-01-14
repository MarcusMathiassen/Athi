#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include <cstdint>

enum buffer_usage
{
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

template <class T>
class VertexBuffer {
private:
  std::uint32_t vbo;
  std::size_t size;
  T* data;

public:
  VertexBuffer() = default;

  void init(T* data, std::size_t size, buffer_usage usage) noexcept {
    this->size = size;
    this->data = data;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data[0]) * size, &data[0], usage);
  }

  void bind() const noexcept;
};
