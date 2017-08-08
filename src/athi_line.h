#pragma once

#include "athi_typedefs.h"
#include "athi_transform.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

struct Athi_Line
{
  Transform transform;
  vec2 pos{0,0};
  f32 width;
  f32 height;
  vec4 color{1.0f,1.0f,1.0f,1.0f};

  Athi_Line() = default;
  void draw() const {}
};

struct Athi_Line_Manager
{
  const u16 indices[6]{0,1,2, 0,2,3};
  enum {TRANSFORM, COLOR, NUM_UNIFORMS};
  enum {POSITION, INDICES, NUM_BUFFERS};

  u32 VAO;
  u32 VBO[NUM_BUFFERS];
  u32 shader_program;
  u32 uniform[NUM_UNIFORMS];

  Athi_Line_Manager() = default;
  ~Athi_Line_Manager();

  void init();
  void update();
  void draw();
};

void add_line(Athi_Line* line);
void init_line_manager();
void draw_rects();

void draw_line(const vec2& p1, const vec2& p2, f32 width, const vec4& color);

extern std::vector<Athi_Line*> line_buffer;
extern Athi_Line_Manager athi_line_manager;
