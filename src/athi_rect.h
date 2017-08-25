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
  vec2 pos;
  f32 width,height;
  vec4 color{1.0f,1.0f,1.0f,1.0f};
  Rect(const vec2& _min, const vec2& _max)
  {
    min = _min;
    max = _max;
    pos = _min;
  }
  bool contain_rect(const Rect &r) const;    
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

  GLenum draw_mode{GL_TRIANGLES};

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
void draw_rect(const vec2& min, f32 width, f32 height, const vec4& color, GLenum draw_type);
void draw_rect(const vec2& min, const vec2& max, const vec4& color, GLenum draw_type);
void draw_hollow_rect(const vec2& min, const vec2& max, const vec4& color);
void draw_rects();

void draw_line(const vec2& p1, const vec2& p2, f32 width, const vec4& color);

extern std::vector<Athi_Rect> rect_immediate_buffer;
extern std::vector<Athi_Rect*> rect_buffer;
extern Athi_Rect_Manager athi_rect_manager;
