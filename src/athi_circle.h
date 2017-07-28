#pragma once

#include "athi_typedefs.h"
#include <glm/glm.hpp>

struct Athi_Circle
{
  u32 id;
  glm::vec2 pos,vel;
  glm::vec4 color;
  f32 radius;
  f32 mass;

  void update();
  void draw();

  void borderCollision();
};
