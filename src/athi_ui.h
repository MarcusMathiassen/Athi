#pragma once

#include "athi_settings.h"
#include "athi_text.h"
#include "athi_typedefs.h"
#include <memory>
#include <vector>

#include <iostream>

struct Athi_UI {
  virtual void update() {}
  virtual void draw() const {}
  virtual ~Athi_UI() = default;
};

struct Athi_UI_Manager {
  f32 scale{1.0f};

  std::vector<Athi_UI *> ui_buffer;

  void update() {
    for (auto &ui : ui_buffer)
      ui->update();
  }

  void draw() const {
    draw_all_text();
    for (const auto &ui : ui_buffer)
      ui->draw();
  }
};

static Athi_UI_Manager athi_ui_manager;

static void update_UI() { athi_ui_manager.update(); }

static void draw_UI() { athi_ui_manager.draw();}


