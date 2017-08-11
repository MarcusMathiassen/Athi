#pragma once

#include "athi_typedefs.h"
#include "athi_utility.h"
#include "athi_line.h"
#include "athi_circle.h"
#include "athi_settings.h"

#include <cmath>
#include <iostream>

struct Athi_Spring
{
  Athi_Circle *object;
  Athi_Circle *anchor;
  f32 length;
  f32 k{0.01f};
  f32 b{0.5f};
  vec4 color{pastel_green};

  void update()
  {
    // Local variables
    const vec2 p1 = object->pos;
    const vec2 p2 = anchor->pos;
    const vec2 v1 = object->vel;
    const vec2 v2 = anchor->vel;
    const f32  m1 = object->mass;
    const f32  m2 = anchor->mass;

    const f32 distance = glm::distance(p1, p2);

    //if (distance < length || distance > length) return;

    const f32  x = length - distance;
    const vec2 x_n1 = glm::normalize(p2-p1);
    const vec2 x_n2 = glm::normalize(p1-p2);
    const vec2 v = v2 - v1;

    // Calculate forces
    const vec2 F1 = -k*x*x_n1 - b*v;
    const vec2 F2 = -k*x*x_n2 - b*v;

    std::cout << "F1: " << F1.x << " " << F1.y<< std::endl;
    std::cout << "F2: " << F2.x << " " << F2.y<< std::endl;

    // Update acceleration
    //object->acc += F1/m1;
    //anchor->acc += F2/m2;
  }

  void draw()
  {
    draw_line(object->pos, anchor->pos, 0.3f, color);
  }
};

extern vector<unique_ptr<Athi_Spring> > spring_buffer;
void attach_spring(Athi_Circle &a, Athi_Circle &b);
void draw_springs();
void update_springs();
void add_spring(const Athi_Spring& s);
