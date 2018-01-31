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

  u32 attrib_counter = 0;
  for (auto& [name, vbo] : vbos) {

    console->info("name: {}, data_members: {}, data_size: {}, is_matrix: {}", 
      name, vbo.data_members, vbo.data_size, vbo.is_matrix);

    glGenBuffers(1, &vbo.handle);
    glBindBuffer(vbo.type, vbo.handle);

    if (vbo.data != nullptr) {
      glBufferData(vbo.type, vbo.data_size, vbo.data, vbo.usage);
    }

    if (!vbo.is_matrix) {
      glEnableVertexAttribArray(attrib_counter);
      glVertexAttribPointer(attrib_counter, vbo.data_members, GL_FLOAT, GL_FALSE, vbo.stride, (void*)vbo.pointer);
      if (vbo.divisor) glVertexAttribDivisor(attrib_counter, vbo.divisor);
      ++attrib_counter;
    } else {
      for (u32 i = 0; i < vbo.data_members; ++i) {
        glEnableVertexAttribArray(i + attrib_counter);
        glVertexAttribPointer(i + attrib_counter, vbo.data_members, GL_FLOAT, GL_FALSE, vbo.stride, (void*)(i * vbo.pointer));
        if (vbo.divisor) glVertexAttribDivisor(i + attrib_counter, vbo.divisor);
      }
      attrib_counter += vbo.data_members;
    }
    console->info("attrib_counter: {}", attrib_counter);
  }
}