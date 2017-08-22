#pragma once

#include "athi_typedefs.h"
#include "athi_utility.h"
#include "athi_line.h"

#include <cmath>
#ifndef M_PI
  #define M_PI 3.14159265359
#endif

#define SOFT_BODY_DEFAULT_NUM_VERTICES 12

struct Athi_SoftBody
{
  // Normal stats
  vec2 pos;
  f32  scale{0.3f};
  vec2 vel;
  vec4 color;

  // Number of points
  vec2 vertices[SOFT_BODY_DEFAULT_NUM_VERTICES];

  void init()
  {
    // Initialize vertices
    for (u32 i = 0; i < SOFT_BODY_DEFAULT_NUM_VERTICES; ++i)
    {
      vertices[i] = vec2(std::cos(i * M_PI * 2.0f / SOFT_BODY_DEFAULT_NUM_VERTICES)*scale, sin(i * M_PI * 2.0f / SOFT_BODY_DEFAULT_NUM_VERTICES)*scale);
    }
  }

  void update()
  {
    // Update vertices
    int i = 4;
    f32 amount = 0.8f;
    vertices[i] = vec2(std::cos(i * M_PI * 2.0f / SOFT_BODY_DEFAULT_NUM_VERTICES)*scale*amount, sin(i * M_PI * 2.0f / SOFT_BODY_DEFAULT_NUM_VERTICES)*scale*amount);
  }

  void draw()
  {
    for (u32 i = 0; i < SOFT_BODY_DEFAULT_NUM_VERTICES; ++i)
    {
      // draw a line to the center
      draw_line(pos, vertices[i], 0.03f, pastel_red);

      // draw a line between vertices
      if (i < SOFT_BODY_DEFAULT_NUM_VERTICES-1) draw_line(vertices[i], vertices[i+1], 0.03f, color);
    }
    // draw a line between the remainding vertices
    draw_line(vertices[0], vertices[SOFT_BODY_DEFAULT_NUM_VERTICES-1], 0.03f, color);
  }

  Athi_SoftBody() = default;
};
