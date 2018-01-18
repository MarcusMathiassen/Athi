#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "athi_camera.h"
#include "athi_transform.h"
#include "athi_shader.h"
#include "athi_typedefs.h"
#include "athi_utility.h"

struct Athi_Line {
  vec2 p1, p2;
  f32 width;
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Transform transform;
  Athi_Line() = default;
};

struct Athi_Line_Manager {
  enum { POSITIONS, COLOR, NUM_UNIFORMS };

  u32 VAO;
  Shader shader;
  u32 shader_program;
  u32 uniform[NUM_UNIFORMS];

  Athi_Line_Manager() = default;
  ~Athi_Line_Manager();

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
