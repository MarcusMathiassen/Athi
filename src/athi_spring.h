#pragma once

#include "athi_line.h"
#include "athi_settings.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

template <class T> struct Athi_Spring {
  T *object;
  T *anchor;
  float length;
  float k{0.2f};
  float b{1.0f};
  glm::vec4 color{pastel_green};

  void update() {

    const glm::vec2 p1 = object->pos;
    const glm::vec2 p2 = anchor->pos;
    const glm::vec2 v1 = object->vel;
    const glm::vec2 v2 = anchor->vel;
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
    object->acc = F1 / m1 * static_cast<float>(timestep);
    anchor->acc = F2 / m2 * static_cast<float>(timestep);
  }

  void draw() const { draw_line(object->pos, anchor->pos, 0.3f, color); }
};
