#include "athi_buffer.h"
#include "opengl_utility.h"

Buffer::~Buffer() {
  glDeleteVertexArrays(1, &vao);
  check_gl_error();
  for (auto & [ name, vbo ] : vbos) glDeleteBuffers(1, &vbo.handle);
  check_gl_error();
}

void Buffer::update(const string& name, void* data, size_t data_size) noexcept {
  glBindVertexArray(vao);  
  check_gl_error();
  auto &vbo = vbos.at(name);
  glBindBuffer(vbo.type, vbo.handle);  check_gl_error();
  if (data_size > vbo.data_size) {
    glBufferData(vbo.type, data_size, data, vbo.usage);  
    check_gl_error();
    vbo.data_size = data_size;
  } else {
    glBufferSubData(vbo.type, 0, vbo.data_size, data);  
    check_gl_error();
  }
}

void Buffer::bind() const noexcept {
  glBindVertexArray(vao);  
  check_gl_error();
}

void Buffer::finish() noexcept {

  glGenVertexArrays(1, &vao); 
  check_gl_error();
  glBindVertexArray(vao);  
  check_gl_error();

  for (auto& [name, vbo] : vbos) {

    glGenBuffers(1, &vbo.handle);  
    check_gl_error();
    glBindBuffer(vbo.type, vbo.handle);  
    check_gl_error();

    if (vbo.data != nullptr) {
      glBufferData(vbo.type, vbo.data_size, vbo.data, vbo.usage);  
      check_gl_error();
    }

    if (!vbo.is_matrix) {
      glEnableVertexAttribArray(vbo.attrib_num);   
      check_gl_error();
      glVertexAttribPointer(vbo.attrib_num, vbo.data_members, GL_FLOAT, GL_FALSE, vbo.stride, (void*)(vbo.pointer));   
      check_gl_error();
      if (vbo.divisor) glVertexAttribDivisor(vbo.attrib_num, vbo.divisor);   
      check_gl_error();
    } else {
      for (u32 i = 0; i < vbo.data_members; ++i) {
        glEnableVertexAttribArray(i + vbo.attrib_num);  
        check_gl_error();
        glVertexAttribPointer(i + vbo.attrib_num, vbo.data_members, GL_FLOAT, GL_FALSE, vbo.stride, (void*)(i * vbo.pointer));  
        check_gl_error();
        if (vbo.divisor) glVertexAttribDivisor(i + vbo.attrib_num, vbo.divisor);  
        check_gl_error();
      }
    }
  }
}