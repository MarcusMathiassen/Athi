#pragma once

#include "../athi_typedefs.h"

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

  bool has_indices = false;
  primitive type{primitive::triangles};
  s32 first{0};
  s32 count{0};
  s32 primitive_count{-1};

  void commit() const noexcept {
    // Are we instanced drawing?
    if (primitive_count != -1) {
      if (has_indices) {
        glDrawElementsInstanced(type, count, GL_UNSIGNED_SHORT, NULL, primitive_count);
      } else {
        glDrawArraysInstanced(type, first, count, primitive_count);
      }
    } else {
      if (has_indices) {
        glDrawElements(type, count, GL_UNSIGNED_SHORT, NULL);
      } else {
        glDrawArrays(type, first, count);
      }
    }
  }
};
