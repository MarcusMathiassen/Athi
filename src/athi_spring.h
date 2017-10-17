#pragma once

#include "athi_circle.h"
#include "athi_line.h"
#include "athi_settings.h"
#include "athi_typedefs.h"
#include "athi_utility.h"

#include <cmath>
#include <iostream>

struct Athi_Spring {
  Athi_Circle *object;
  Athi_Circle *anchor;
  f32 length;
  f32 k{0.2f};
  f32 b{1.0f};
  vec4 color{pastel_green};

  void update() {
    // Local variables
    const vec2 p1 = object->pos;
    const vec2 p2 = anchor->pos;
    const vec2 v1 = object->vel;
    const vec2 v2 = anchor->vel;
    const f32 m1 = object->mass;
    const f32 m2 = anchor->mass;

    const f32 distance = glm::distance(p1, p2);

    const f32 x = length - distance;
    const vec2 x_n1 = glm::normalize(p2 - p1);
    const vec2 x_n2 = glm::normalize(p1 - p2);

    // Calculate forces
    const vec2 F1 = -k * x * x_n1 - b * v1;
    const vec2 F2 = -k * x * x_n2 - b * v2;

    // Update acceleration
    object->acc = F1 / m1 * 0.0001f * (f32)timestep;
    anchor->acc = F2 / m2 * 0.0001f * (f32)timestep;
  }

  void draw() const { draw_line(object->pos, anchor->pos, 0.3f, color); }
};

extern vector<unique_ptr<Athi_Spring> > spring_buffer;
void attach_spring(Athi_Circle &a, Athi_Circle &b);
void draw_springs();
void update_springs();
void add_spring(const Athi_Spring &s);
