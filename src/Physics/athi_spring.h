#pragma once

#include "../athi_typedefs.h"

#include "../Utility/athi_globals.h" // colors
#include "athi_line.h"  // draw_line

template <class T>
struct Athi_Spring {
  T *object;
  T *anchor;
  f32 length;
  f32 k{0.2f};
  f32 b{1.0f};
  vec4 color{pastel_green};

  void update(float dt) {
    const auto p1 = object->pos;
    const auto p2 = anchor->pos;
    const auto v1 = object->vel;
    const auto v2 = anchor->vel;
    const auto m1 = object->mass;
    const auto m2 = anchor->mass;

    const auto distance = glm::distance(p1, p2);

    const auto x = length - distance;
    const auto x_n1 = glm::normalize(p2 - p1);
    const auto x_n2 = glm::normalize(p1 - p2);

    // Calculate forces
    const auto F1 = -k * x * x_n1 - b * v1;
    const auto F2 = -k * x * x_n2 - b * v2;

    // Update acceleration
    object->acc = F1 / m1 * dt;
    anchor->acc = F2 / m2 * dt;
  }

  void draw() const { draw_line(object->pos, anchor->pos, 0.3f, color); }
};
