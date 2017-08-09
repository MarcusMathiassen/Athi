#pragma once

#include "athi_typedefs.h"

struct Athi_Particle
{
  vec2 pos;
  vec2 vel;
};

extern vector<unique_ptr<Athi_Particle> > particle_buffer;

struct Athi_Particle_Manager
{
  vector<mat4> transforms;
  vector<vec4> colors;

  size_t transform_bytes_allocated{0};
  size_t color_bytes_allocated{0};


  void init();
  void update();
  void draw();
};
