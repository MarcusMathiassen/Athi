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
#include "opengl_utility.h" //   check_gl_error();

#define GLEW_STATIC
#include <GL/glew.h>

enum primitive {
  points = GL_POINTS,
  line_strip = GL_LINE_STRIP,
  line_loop = GL_LINE_LOOP,
  lines = GL_LINES,
  line_strip_adjacency = GL_LINE_STRIP_ADJACENCY,
  lines_adjacency = GL_LINES_ADJACENCY,
  triangle_strip = GL_TRIANGLE_STRIP,
  triangle_fan = GL_TRIANGLE_FAN,
  triangles = GL_TRIANGLES,
  triangle_strip_adjacency = GL_TRIANGLE_STRIP_ADJACENCY,
  triangles_adjacency = GL_TRIANGLES_ADJACENCY,
  patches = GL_PATCHES,
};

struct CommandBuffer {

  primitive type{primitive::triangles};
  s32 count{0};
  s32 first{0};
  s32 primitive_count{-1};
  bool has_indices = false;

  void commit() const noexcept {
    // Are we instanced drawing?
    if (primitive_count != -1) {
      if (has_indices) {
        glDrawElementsInstanced(type, count, GL_UNSIGNED_SHORT, NULL, primitive_count);
        check_gl_error();
      } else {
        glDrawArraysInstanced(type, first, count, primitive_count);
        check_gl_error();
      }
    } else {
      if (has_indices) {
        glDrawElements(type, count, GL_UNSIGNED_SHORT, NULL);
        check_gl_error();
      } else {
        glDrawArrays(type, first, count);
        check_gl_error();
      }
    }
  }
};
