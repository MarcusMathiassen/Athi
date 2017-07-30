#pragma once

#include "athi_typedefs.h"
#include "athi_rect.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

template <typename T>
struct Athi_Slider
{
  T* var;

  vec2 pos;
  u32 range_min, range_max;
  u32 step_per_tick;

  u32 steps{600};

  enum { HOVER, ACTIVE, PRESSED, NOTHING};

  Athi_Rect slider_border;
  Athi_Rect slider_input_box;

  Athi_Slider(T* v) : var(v) {}


  void init()
  {

    // Border
    slider_border.pos = pos;
    slider_border.color = vec4(0.3f,0.3f,0.3f,1.0f);
    slider_border.width  = 0.3f;
    slider_border.height = 0.05f;
    slider_border.init();

    // Box
    slider_input_box.pos = pos;
    slider_input_box.color = vec4(0.9f, 0.3f, 0.3f,1.0f);
    slider_input_box.width = 0.05f;
    slider_input_box.pos.x = slider_border.pos.x + *var * slider_border.pos.x;
    slider_input_box.height = 0.05f;
    slider_input_box.init();

    step_per_tick = range_max-range_min / steps;
  }

  void update()
  {

    switch(get_status())
    {
      case HOVER: slider_input_box.color = vec4(1.0f, 0.5f, 0.5f,1.0f); break;
      case ACTIVE: slider_input_box.color = vec4(1,0,1,1); break;
      case PRESSED: slider_input_box.color = vec4(0.5f, 0.3f, 0.3f,1.0f); break;
      case NOTHING: slider_input_box.color = vec4(0.9f, 0.3f, 0.3f,1.0f); break;
    }
  }

  void draw()
  {
    slider_border.draw();
    slider_input_box.draw();
  }

  u32 get_status()
  {
    f64 mouse_x, mouse_y;
    glfwGetCursorPos(glfwGetCurrentContext(), &mouse_x, &mouse_y);

    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    mouse_x = -1.0f + 2 * mouse_x / width;
    mouse_y = +1.0f - 2 * mouse_y / height;

    if (
      mouse_x > slider_border.pos.x && mouse_x < slider_border.pos.x+slider_border.width &&
      mouse_y > slider_border.pos.y && mouse_y < slider_border.pos.y+slider_border.height)
    {
      int state = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT);
      if (state == GLFW_PRESS)
        {
          // move the box
          slider_input_box.pos.x = mouse_x - (slider_input_box.width*0.5f);

          std::cout << "min: " << slider_border.pos.x << std::endl;
          std::cout << "max: " << slider_border.pos.x + slider_border.width << std::endl;
          std::cout << "difference: " << (slider_border.pos.x + slider_border.width) - slider_border.pos.x << std::endl;

          f32 percent_one = (((slider_border.pos.x+slider_input_box.width*0.5f) + (slider_border.width-slider_input_box.width*0.5f)) - slider_border.pos.x)/100.0f;
          std::cout << "1% is: " << percent_one << std::endl;

          f32 box_at = slider_input_box.pos.x + (slider_input_box.width*0.5f);
          std::cout << "box is at " << box_at << std::endl;

          std::cout << "how many percent is the box at? " << box_at / percent_one << std::endl;

          std::cout << "limit: " << (box_at/percent_one)/100.0f*range_max << std::endl;
          *var = ((box_at/percent_one)/100.0f)*range_max;

          return PRESSED;
        }
      return HOVER;
    }
    return NOTHING;
  }
};
