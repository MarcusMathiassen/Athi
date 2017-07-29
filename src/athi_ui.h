#pragma once

#include "athi_typedefs.h"
#include <vector>
#include <memory>


struct Athi_UI
{
  u32 id;
  virtual void update(){}
  virtual void draw() const{}
};

struct Athi_UI_Manager
{
  f32 scale{1.0f};
  std::vector<std::unique_ptr<Athi_UI> > ui_buffer;

  void update()
  {
    for (auto &ui: ui_buffer)
    {
      ui->update();
    }
  }

  void draw() const
  {
    glViewport(0.0f, 0.0f, 1000.0f*scale, 1000.0f*scale);
    for (const auto &ui: ui_buffer)
    {
      ui->draw();
    }
  }

  void add_ui(const Athi_UI& ui)
  {
    ui_buffer.emplace_back(std::make_unique<Athi_UI>(ui));
  }
};
