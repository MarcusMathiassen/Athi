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

#include "athi_buffer.h"

Buffer::~Buffer()
{
  glDeleteVertexArrays(1, &vao); check_gl_error();

  // Cleanup bufferss
  for (auto & [ name, vbo ] : vbos) {
    glDeleteBuffers(1, &vbo.handle); check_gl_error();
  }
}


void Buffer::bind() const noexcept
{
  glBindVertexArray(vao); check_gl_error();
}

void Buffer::finish() noexcept {
  glGenVertexArrays(1, &vao); check_gl_error();
  glBindVertexArray(vao); check_gl_error();

  for (auto & [ name, vbo ] : vbos) {
    glGenBuffers(1, &vbo.handle); check_gl_error();
    glBindBuffer(vbo.type, vbo.handle); check_gl_error();

    if (vbo.data != nullptr) {
      glBufferData(vbo.type, vbo.data_size, vbo.data, vbo.usage); check_gl_error();
    }

    if (!vbo.is_matrix) {
      glEnableVertexAttribArray(vbo.attrib_num); check_gl_error();
      glVertexAttribPointer(vbo.attrib_num, vbo.data_members, GL_FLOAT, GL_FALSE, vbo.stride, (void*)(vbo.pointer)); check_gl_error();
      if (vbo.divisor) glVertexAttribDivisor(vbo.attrib_num, vbo.divisor); check_gl_error();
    } else {
      for (u32 i = 0; i < vbo.data_members; ++i) {
        glEnableVertexAttribArray(i + vbo.attrib_num); check_gl_error();
        glVertexAttribPointer(i + vbo.attrib_num, vbo.data_members, GL_FLOAT, GL_FALSE, vbo.stride, (void*)(i * vbo.pointer)); check_gl_error();
        if (vbo.divisor) glVertexAttribDivisor(i + vbo.attrib_num, vbo.divisor); check_gl_error();
      }
    }
  }
}
