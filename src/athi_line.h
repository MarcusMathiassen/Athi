#pragma once
#include "athi_typedefs.h"

#include "athi_transform.h"   // Transform
#include "./Renderer/athi_shader.h"      // Shader
#include "./Renderer/athi_buffer.h"      // GPUBuffer

#define GLEW_STATIC
#include <GL/glew.h>

struct Athi_Line {
  vec2 p1, p2;
  f32 width;
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Transform transform;
  Athi_Line() = default;
};

struct Athi_Line_Manager {
  Shader shader;
  GPUBuffer gpu_buffer;

  Athi_Line_Manager() = default;
  void init();
  void update();
  void draw();
};

void init_line_manager();
void add_line(Athi_Line *line);
void init_line_manager();
void draw_lines();

void draw_line(const vec2 &p1, const vec2 &p2, f32 width, const vec4 &color);

extern std::vector<Athi_Line> line_immediate_buffer;
extern std::vector<Athi_Line *> line_buffer;
extern Athi_Line_Manager athi_line_manager;
