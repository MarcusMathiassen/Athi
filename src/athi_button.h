#pragma once

#include "athi_typedefs.h"
#include "athi_rect.h"
#include "athi_text.h"

struct Athi_Button
{
  vec2 pos;
  vec4 color;

  Athi_Rect box;
  Athi_Text text;

  Athi_Button() = default;

  void draw()
  {

  }
};
