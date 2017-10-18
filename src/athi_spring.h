#pragma once

#include "athi_circle.h"
#include "athi_line.h"
#include "athi_settings.h"
#include "athi_typedefs.h"
#include "athi_utility.h"

#include <glm/vec4.hpp>
#include <cmath>
#include <iostream>

struct Athi_Spring {
  Athi_Circle *object;
  Athi_Circle *anchor;
  float length;
  float k{0.2f};
  float b{1.0f};
  glm::vec4 color{pastel_green};

  void update() {
    // Local variables
    const vec2 p1 = object->pos;
    const vec2 p2 = anchor->pos;
    const vec2 v1 = object->vel;
    const vec2 v2 = anchor->vel;
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

extern std::vector<std::unique_ptr<Athi_Spring>> spring_buffer;
void attach_spring(Athi_Circle &a, Athi_Circle &b);
void draw_springs();
void update_springs();
void add_spring(const Athi_Spring &s);
