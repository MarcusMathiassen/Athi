#pragma once

#include "athi_typedefs.h"
#include "athi_rect.h"
#include "athi_ui.h"
#include "athi_input.h"
#include "athi_camera.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

// @Not working: ranges are showing wildy different numbers

template <typename T>
class Athi_Slider : public Athi_UI
{
private:
  f32 box_max_pos;
  f32 box_min_pos;

  enum { HOVER, PRESSED, NOTHING};
  u16 last_state{NOTHING};

  T* var;

  Athi_Rect slider_border;
  Athi_Rect slider_input_box;

public:
  vec2 pos{0,0};

  f32 width{1.0f};
  f32 height{0.05f};

  f32 knob_width{0.05f};

  vec4 hover_color{1.0f, 0.5f, 0.5f,1.0f};
  vec4 pressed_color{0.5f, 0.3f, 0.3f,1.0f};
  vec4 idle_color{0.9f, 0.3f, 0.3f,1.0f};

  vec4 outer_box_color{0.3f,0.3f,0.3f,1.0f};
  f32 min{0}, max{100};

  Athi_Slider(T* t) : var(t) {}
  void init()
  {
    // Border
    slider_border.pos = pos;
    slider_border.color = outer_box_color;
    slider_border.width  = width;
    slider_border.height = height;
    slider_border.init();

    // Box
    slider_input_box.pos = pos;
    slider_input_box.color = idle_color;
    slider_input_box.width  = knob_width;
    slider_input_box.height = height;
    slider_input_box.pos.x = slider_border.pos.x + slider_border.width * 0.5f;
    slider_input_box.init();

    box_min_pos = slider_border.pos.x;
    box_max_pos = slider_border.pos.x + slider_border.width - slider_input_box.width;

    // set starting position of box based on the var
    slider_input_box.pos.x = box_min_pos + (box_max_pos - box_min_pos)*((*var) - min) / (max-min);
  }

  void update()
  {
    last_state = get_status();
    switch(last_state)
    {
      case HOVER:   slider_input_box.color = hover_color;   break;
      case PRESSED: slider_input_box.color = pressed_color; break;
      case NOTHING: slider_input_box.color = idle_color;    break;
    }
  }

  void draw() const
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
    const f32 box_pos_to_slider_val = ((slider_input_box.pos.x - box_min_pos) / (box_max_pos - box_min_pos)) * (max-min) + min;
    *var = box_pos_to_slider_val;
  }

  u32 get_status()
  {
    f64 mouse_x, mouse_y;
    glfwGetCursorPos(glfwGetCurrentContext(), &mouse_x, &mouse_y);

    s32 width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    mouse_x = -1.0f + 2 * mouse_x / width;
    mouse_y = +1.0f - 2 * mouse_y / height;

    s32 state = get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS && last_state == PRESSED)
    {
      update_position(mouse_x);
      return PRESSED;
    }

    if (  mouse_x > slider_border.pos.x && mouse_x < slider_border.pos.x+slider_border.width &&
          mouse_y > slider_border.pos.y && mouse_y < slider_border.pos.y+slider_border.height)
    {
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

template <typename T>
auto create_slider(T* t)
{
  auto slider = std::make_unique<Athi_Slider<T>>(t);
  return slider;
}
