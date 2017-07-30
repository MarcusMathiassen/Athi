#pragma once

#include "athi_typedefs.h"
#include "athi_rect.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

template <typename T>
class Athi_Slider
{
private:
  f32 box_max_pos;
  f32 box_min_pos;

  enum { HOVER, ACTIVE, PRESSED, NOTHING};
  u16 last_state{NOTHING};

  T* var;

  f32 percent_one;

  Athi_Rect slider_border;
  Athi_Rect slider_input_box;

public:
  vec2 pos;
  f32 width{1.0f};
  u32 range_min, range_max;

  Athi_Slider(T* v) : var(v) {}


  void init()
  {

    // Border
    slider_border.pos = pos;
    slider_border.color = vec4(0.3f,0.3f,0.3f,1.0f);
    slider_border.width  = 0.5f * width;
    slider_border.height = 0.05f;
    slider_border.init();

    // Box
    slider_input_box.pos = pos;
    slider_input_box.color = vec4(0.9f, 0.3f, 0.3f,1.0f);
    slider_input_box.width = 0.05f;
    slider_input_box.pos.x = slider_border.pos.x + slider_border.width*0.5f;
    slider_input_box.height = slider_border.height;
    slider_input_box.init();

    box_min_pos = slider_border.pos.x;
    box_max_pos = slider_border.pos.x + slider_border.width - slider_input_box.width;
  }

  void update()
  {
    last_state = get_status();
    switch(last_state)
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

  void update_position(f32 mouse_x)
  {
    // Move box to where the mouse is.
    slider_input_box.pos.x = mouse_x - (slider_input_box.width*0.5f);

    // make sure it stays within the border
    if (slider_input_box.pos.x < box_min_pos) slider_input_box.pos.x = box_min_pos;
    if (slider_input_box.pos.x > box_max_pos) slider_input_box.pos.x = box_max_pos;

    // Get the percentage of box position.
    const f32 box_at_perc = ((slider_input_box.pos.x - box_min_pos) / (box_max_pos - box_min_pos)) * 100;
    *var = box_at_perc * ((range_max-range_min)/100.0f);
  }

  u32 get_status()
  {
    f64 mouse_x, mouse_y;
    glfwGetCursorPos(glfwGetCurrentContext(), &mouse_x, &mouse_y);

    int width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    mouse_x = -1.0f + 2 * mouse_x / width;
    mouse_y = +1.0f - 2 * mouse_y / height;

    int state = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS && last_state == PRESSED)
    {
      update_position(mouse_x);
      return PRESSED;
    }

    if (
      mouse_x > slider_border.pos.x && mouse_x < slider_border.pos.x+slider_border.width &&
      mouse_y > slider_border.pos.y && mouse_y < slider_border.pos.y+slider_border.height)
    {
      int state = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT);
      if (state == GLFW_PRESS)
      {
        update_position(mouse_x);
        return PRESSED;
      }
      return HOVER;
    }
    return NOTHING;
  }
};
