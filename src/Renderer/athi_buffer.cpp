#include "athi_buffer.h"

Buffer::~Buffer() {
  glDeleteVertexArrays(1, &vao);
  for (auto & [ name, vbo ] : vbos) glDeleteBuffers(1, &vbo.handle);
}

void Buffer::update(const string& name, void* data, size_t data_size) noexcept {
  glBindVertexArray(vao);
  auto &vbo = vbos.at(name);
  glBindBuffer(vbo.type, vbo.handle);
  if (data_size > vbo.data_size) {
    glBufferData(vbo.type, data_size, data, vbo.usage);
    vbo.data_size = data_size;
  } else {
    glBufferSubData(vbo.type, 0, vbo.data_size, data);
  }
}

void Buffer::bind() const noexcept {
  glBindVertexArray(vao);
}

void Buffer::finish() noexcept {

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  for (auto& [name, vbo] : vbos) {

    glGenBuffers(1, &vbo.handle);
    glBindBuffer(vbo.type, vbo.handle);

    if (vbo.data != nullptr) {
      glBufferData(vbo.type, vbo.data_size, vbo.data, vbo.usage);
    }

    if (!vbo.is_matrix) {
      glEnableVertexAttribArray(vbo.attrib_num);
      glVertexAttribPointer(vbo.attrib_num, vbo.data_members, GL_FLOAT, GL_FALSE, vbo.stride, (void*)vbo.pointer);
      if (vbo.divisor) glVertexAttribDivisor(vbo.attrib_num, vbo.divisor);
    } else {
      for (u32 i = 0; i < vbo.data_members; ++i) {
        glEnableVertexAttribArray(i + vbo.attrib_num);
        glVertexAttribPointer(i + vbo.attrib_num, vbo.data_members, GL_FLOAT, GL_FALSE, vbo.stride, (void*)(i * vbo.pointer));
        if (vbo.divisor) glVertexAttribDivisor(i + vbo.attrib_num, vbo.divisor);
      }
    }
  }
}