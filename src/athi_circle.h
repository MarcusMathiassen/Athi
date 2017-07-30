#pragma once

#include "athi_typedefs.h"

#include "athi_utility.h"
#include "athi_transform.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

struct Athi_Circle
{
  vec2 pos{0, 0};
  vec2 vel{0, 0};

  f32 radius{0.1};
  f32 mass{1};

  vec4 color{1, 1, 1, 1};

  void update();
  void draw() const;

  Athi_Circle() = default;
};


struct Athi_Circle_Manager
{
  enum { POSITION, COLOR, TRANSFORM, NUM_BUFFERS };


  std::vector<Athi_Circle> circle_buffer;

  std::vector<mat4> transforms;
  std::vector<vec4> colors;

  // Mesh
  GLuint VAO;
  GLuint VBO[NUM_BUFFERS];
  size_t transform_bytes_allocated{0};
  size_t color_bytes_allocated{0};

  // Shader
  u32 m_shaderProgram;

  Athi_Circle_Manager() = default;
};
