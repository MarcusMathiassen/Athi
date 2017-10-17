#pragma once

#include "athi_input.h"
#include "athi_rect.h"
#include "athi_text.h"
#include "athi_typedefs.h"
#include "athi_ui.h"

#include <iostream>

#include <memory>

struct Athi_Checkbox : public Athi_UI {
  enum { HOVER, PRESSED, IDLE, TOGGLE };
  u16 last_state{IDLE};

  bool *variable;
  bool *active_if{nullptr};
  vec2 pos;

  Athi_Text text;

  f32 width{0.03f};
  f32 height{0.03f};

  vec4 box_color{0.3f, 0.3f, 0.3f, 1.0f};

  vec4 hover_color{1.0f, 0.5f, 0.5f, 1.0f};
  vec4 pressed_color{0.5f, 0.3f, 0.3f, 1.0f};
  vec4 idle_color{0.9f, 0.3f, 0.3f, 1.0f};
  vec4 active_color{0.9f, 0.3f, 0.3f, 1.0f};

  Athi::Rect box;

  void draw() const;
  void update();
  bool hover_over();
  u32 get_status();
  void init();
};

static void add_checkbox(Athi_Checkbox *ui) {
  ui->init();
  athi_ui_manager.ui_buffer.emplace_back(ui);
}
