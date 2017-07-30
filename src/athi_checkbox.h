#pragma once

#include "athi_typedefs.h"
#include "athi_rect.h"
#include "athi_input.h"

#include <iostream>

// @Buggy: too fast

struct Athi_Checkbox
{

  enum { HOVER, PRESSED, IDLE, TOGGLE};
  u16 last_state{IDLE};

  bool* variable;
  vec2 pos;

  f32 width{0.05f};
  f32 height{0.05f};

  vec4 outer_box_color{0.3f,0.3f,0.3f,1.0f};
  vec4 inner_box_color;

  vec4 hover_color{1.0f, 0.5f, 0.5f,1.0f};
  vec4 pressed_color{0.5f, 0.3f, 0.3f,1.0f};
  vec4 idle_color{0.9f, 0.3f, 0.3f,1.0f};

  Athi_Rect outer_box;
  Athi_Rect inner_box;

  void update()
  {
    if (last_state == PRESSED && get_status() == HOVER) last_state = TOGGLE;
    switch(last_state)
    {
      case HOVER:       inner_box.color = hover_color;   break;
      case PRESSED:     inner_box.color = pressed_color; break;
      case IDLE:        inner_box.color = idle_color;    break;
      case TOGGLE:      inner_box.color = pressed_color; *variable = !(*variable); break;
    }
    last_state = get_status();
    std::cout << "last_state: " << last_state << std::endl;
  }

  void draw() const
  {
    outer_box.draw();
    if (*variable) inner_box.draw();
  }


  bool hover_over()
  {
    f64 mouse_x, mouse_y;
    glfwGetCursorPos(glfwGetCurrentContext(), &mouse_x, &mouse_y);

    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    mouse_x = -1.0f + 2 * mouse_x / width;
    mouse_y = +1.0f - 2 * mouse_y / height;

    if (
      mouse_x > outer_box.pos.x && mouse_x < outer_box.pos.x+outer_box.width &&
      mouse_y > outer_box.pos.y && mouse_y < outer_box.pos.y+outer_box.height)
    {
      return true;
    }
    return false;
  }

    u32 get_status()
  {
    if (hover_over())
    {
      if (get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) return PRESSED;
      return HOVER;
    }
    return IDLE;
  }

  void init()
  {
        // Border
    outer_box.pos = pos;
    outer_box.color = outer_box_color;
    outer_box.width  = width;
    outer_box.height = height;
    outer_box.init();

    // Box
    inner_box.pos = pos;
    inner_box.color = inner_box_color;
    inner_box.width  = width;
    inner_box.height = height;
    inner_box.init();

  }
};
