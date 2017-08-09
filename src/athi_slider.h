#pragma once

#include "athi_camera.h"
#include "athi_input.h"
#include "athi_rect.h"
#include "athi_settings.h"
#include "athi_text.h"
#include "athi_typedefs.h"
#include "athi_ui.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

template <typename T>
class Athi_Slider : public Athi_UI {
private:
  f32 box_max_pos;
  f32 box_min_pos;

  enum { HOVER, PRESSED, NOTHING };
  u16 last_state{NOTHING};

  Rect slider_box;
  Rect slider_knob;
  Rect variable_indicator_box;

  Athi_Text text;

public:
  std::string str;
  vec2 pos{0, 0};


  T *var{nullptr};
  T *var_indicator{nullptr};
  bool *active_if{nullptr};
  f32 width{1.0f};
  f32 height{0.03f};

  f32 knob_width{0.05f};

  vec4 hover_color{1.0f, 0.5f, 0.5f, 1.0f};
  vec4 pressed_color{0.5f, 0.3f, 0.3f, 1.0f};
  vec4 idle_color{0.9f, 0.3f, 0.3f, 1.0f};
  vec4 indicator_color{0.8f, 0.8f, 0.8f, 1.0f};
  vec4 indicator_target_color{0.1f, 0.9f, 0.1f, 1.0f};

  vec4 outer_box_color{0.3f, 0.3f, 0.3f, 1.0f};
  f32 min{0}, max{100};

  Athi_Slider() = default;
  Athi_Slider(T *t) : var(t) {}
  void init() {
    // Text description
    text.pos = pos;
    text.pos.y += height;
    // add_text(&text);

    // Border
    slider_box.pos = pos;
    slider_box.color = outer_box_color;
    slider_box.width = width;
    slider_box.height = height;
    // add_rect(&slider_box);

    // Box
    slider_knob.pos = pos;
    slider_knob.color = idle_color;
    slider_knob.width = knob_width;
    slider_knob.height = height;
    // add_rect(&slider_knob);

    box_min_pos = slider_box.pos.x;
    box_max_pos = slider_box.pos.x + slider_box.width - slider_knob.width;

    // set starting position of box based on the var
    slider_knob.pos.x = box_min_pos + (box_max_pos - box_min_pos) *
                                          ((*var) - min) / (max - min);

    if (var_indicator != nullptr) {
      variable_indicator_box.pos = slider_knob.pos;
      variable_indicator_box.color = indicator_color;
      variable_indicator_box.width = knob_width * 0.25f;
      variable_indicator_box.height = height;
      // add_rect(&variable_indicator_box);
    }
  }

  void update()
  {
    if (active_if != nullptr) if (!(*active_if)) return;

    // Update text variable
    std::string temp = str + std::to_string(*var);
    text.str = temp;

    if (var_indicator != nullptr) {
      // update its position
      variable_indicator_box.pos.x =
          variable_indicator_box.width * 1.5f + box_min_pos +
          (box_max_pos - box_min_pos) * ((*var_indicator) - min) / (max - min);

      // change color to green if inside knob
      if (variable_indicator_box.pos.x > slider_knob.pos.x &&
          variable_indicator_box.pos.x + variable_indicator_box.width <
              slider_knob.pos.x + slider_knob.width) {
        variable_indicator_box.color = indicator_target_color;
      } else
        variable_indicator_box.color = indicator_color;
    }

    if (!mouse_busy_UI)
    {
      last_state = get_status();
      switch (last_state) {
      case HOVER:
        slider_knob.color = hover_color;
        break;
      case PRESSED:
        slider_knob.color = pressed_color;
        break;
      case NOTHING:
        slider_knob.color = idle_color;
        break;
      }
    }
  }

  void draw() const {
    if (active_if != nullptr) if (!(*active_if)) return;
    draw_rect(slider_box.pos, slider_box.width, slider_box.height,
              slider_box.color, GL_TRIANGLES);

    draw_rect(slider_knob.pos, slider_knob.width, slider_knob.height,
              slider_knob.color, GL_TRIANGLES);

    draw_rect(variable_indicator_box.pos, variable_indicator_box.width,
              variable_indicator_box.height, variable_indicator_box.color,
              GL_TRIANGLES);

    draw_text(text.str, text.pos, text.color);
  }

  void update_position(f32 mouse_x) {
    // Move box to where the mouse is.
    slider_knob.pos.x = mouse_x - (slider_knob.width * 0.5f);

    // make sure it stays within the border
    if (slider_knob.pos.x < box_min_pos)
      slider_knob.pos.x = box_min_pos;
    if (slider_knob.pos.x > box_max_pos)
      slider_knob.pos.x = box_max_pos;

    // Get the percentage of box position.
    const f32 box_pos_to_slider_val =
        ((slider_knob.pos.x - box_min_pos) / (box_max_pos - box_min_pos)) *
            (max - min) +
        min;
    *var = box_pos_to_slider_val;
  }

  u32 get_status() {
    f64 mouse_x, mouse_y;
    glfwGetCursorPos(glfwGetCurrentContext(), &mouse_x, &mouse_y);

    s32 width, height;
    glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
    mouse_x = -1.0f + 2 * mouse_x / width;
    mouse_y = +1.0f - 2 * mouse_y / height;

    s32 state = get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS && last_state == PRESSED) {
      update_position(mouse_x);
      return PRESSED;
    }

    if (mouse_x > slider_box.pos.x &&
        mouse_x < slider_box.pos.x + slider_box.width &&
        mouse_y > slider_box.pos.y &&
        mouse_y < slider_box.pos.y + slider_box.height) {
      if (state == GLFW_PRESS) {
        update_position(mouse_x);
        return PRESSED;
      }
      return HOVER;
    }
    return NOTHING;
  }
};

template <typename T> static void add_slider(Athi_Slider<T> *ui) {
  ui->init();
  athi_ui_manager.ui_buffer.emplace_back(ui);
}
