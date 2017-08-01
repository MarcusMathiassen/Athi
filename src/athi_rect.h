#pragma once

#include "athi_typedefs.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct Athi_Rect
{
  vec2 min, max;

  string id;
  static constexpr u16 indices[]{0,1,2, 0,2,3};
  enum {TRANSFORM, COLOR, NUM_UNIFORMS};
  enum {POSITION, INDICES, NUM_BUFFERS};

  u32 VAO;
  u32 VBO[NUM_BUFFERS];
  u32 shader_program;
  u32 uniform[NUM_UNIFORMS];

  vec2 pos;
  f32 width;
  f32 height;
  vec4 color;

  Athi_Rect(const vec2& min, const vec2& max);
  Athi_Rect() = default;
  ~Athi_Rect();
  bool contains(u32 id) const;
  void init();
  void draw(GLenum draw_type = GL_TRIANGLE_FAN) const;
};
