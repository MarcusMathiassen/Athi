#pragma once

#include "athi_typedefs.h"

class Rigid_Body {
 public:
  vec2 pos;
  vec2 vel;
  vec2 acc;

  f32 mass;

  bool kinematic{false};

  virtual void draw(){};
  virtual void update(){};
};
