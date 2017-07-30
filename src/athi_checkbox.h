#pragma once

#include "athi_typedefs.h"
#include "athi_rect.h"
#include "athi_input.h"
#include "athi_text.h"

#include <iostream>

//@Note to self: Fix the colors. They don't look right.

struct Athi_Checkbox
{

  enum { HOVER, PRESSED, IDLE, TOGGLE};
  u16 last_state{IDLE};

  bool* variable;
  vec2 pos;

  Athi_Text description;

  f32 width{0.05f};
  f32 height{0.05f};

  vec4 outer_box_color{0.3f,0.3f,0.3f,1.0f};
  vec4 inner_box_color;

  vec4 hover_color{0.4f,0.4f,0.4f,1.0f};
  vec4 pressed_color{0.5f, 0.3f, 0.3f,1.0f};
  vec4 idle_color{0.9f, 0.3f, 0.3f,1.0f};

  Athi_Rect outer_box;
  Athi_Rect inner_box;

  void draw() const;
  void update();
  bool hover_over();
  u32  get_status();
  void init();

};

// auto create_checkbox(bool* b)
// {
//   auto checkbox = std::make_unique<Athi_Checkbox>(b);
//   return checkbox;
// }
