#pragma once

#include "athi_typedefs.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

static constexpr u16 indices[]{0,1,2, 0,2,3};

struct Athi_Rect
{
  string id;
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

  Athi_Rect() = default;
  ~Athi_Rect();
  void init();
  void draw(GLenum draw_type = GL_TRIANGLE_FAN) const;
};

extern std::vector<Athi_Rect*> rect_buffer;

struct Athi_Rect_Manager
{
  enum {TRANSFORM, COLOR, NUM_UNIFORMS};
  enum {POSITION, INDICES, NUM_BUFFERS};
  
  u32 VAO;
  u32 VBO[NUM_BUFFERS];
  u32 shader_program;
  u32 uniform[NUM_UNIFORMS];
  
  
  Athi_Rect_Manager() = default;
  ~Athi_Rect_Manager();
  
  void init();
  void update();
  void draw();
};

static Athi_Rect_Manager athi_rect_manager;
