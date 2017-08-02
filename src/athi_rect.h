#pragma once

#include "athi_typedefs.h"
#include "athi_transform.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

static constexpr u16 indices[]{0,1,2, 0,2,3};

struct Rect
{
  vec2 min,max;
  vec4 color{1.0f,1.0f,1.0f,1.0f};
  Rect(const vec2& min, const vec2& max) : min(min), max(max) {}
  bool contains(u32 id);
  Rect() = default;
};

struct Athi_Rect
{
  Transform transform;
  vec2 pos{0,0};
  f32 width;
  f32 height;
  vec4 color{1.0f,1.0f,1.0f,1.0f};

  Athi_Rect() = default;
  void draw() const {}
};

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

void add_rect(Athi_Rect* rect);
void init_rect_manager();
void draw_rect(const vec2& min, const vec2& max, const vec4& color, GLenum draw_type);
void draw_rects();

extern std::vector<Athi_Rect*> rect_buffer;
extern Athi_Rect_Manager athi_rect_manager;
