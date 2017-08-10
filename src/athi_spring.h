#pragma once

#include "athi_typedefs.h"
#include "athi_utility.h"
#include "athi_line.h"
#include "athi_circle.h"
#include "athi_settings.h"

#include <iostream>

struct Athi_Spring
{
  Athi_Circle *p1;
  Athi_Circle *p2;
  f32 length;
  f32 k{0.1f};
  f32 b{0.1f};
  vec4 color{pastel_green};

  void update()
  {
    // make sure distance between p1 and p2 is length
    f32 distance = glm::distance(p2->pos, p1->pos);

    vec2 dist = vec2(p2->pos.x-(p1->pos.x), p2->pos.y-(p1->pos.y));
    vec2 v = p2->vel - p1->vel;
    f32 d = (distance < length) ? -1.0f : 1.0f;

    //  F = -kx -bv
    vec2 F = d*k*dist - b*v;

    p1->vel = F;
    //p2->vel = -F;
  }

  void draw()
  {
    draw_line(p1->pos, p2->pos, 0.03f, color);
  }
};

extern vector<unique_ptr<Athi_Spring> > spring_buffer;
void attach_spring(Athi_Circle &a, Athi_Circle &b);
void draw_springs();
void update_springs();
void add_spring(const Athi_Spring& s);
