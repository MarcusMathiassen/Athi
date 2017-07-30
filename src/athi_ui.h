#pragma once

#include "athi_typedefs.h"
#include "athi_text.h"
#include <vector>
#include <memory>

#include <iostream>

struct Athi_UI
{
  virtual void update(){}
  virtual void draw() const{}
};

struct Athi_UI_Manager
{
  f32 scale{1.0f};

  std::vector<std::unique_ptr<Athi_UI>>  ui_buffer;

  void update()
  {
    for (auto &ui: ui_buffer)  ui->update();
  }

  void draw() const
  {
    //glViewport(0.0f, 0.0f, 1000.0f*scale, 1000.0f*scale);

    for (const auto &ui: ui_buffer) ui->draw();

    int width, height;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
    glViewport(0, 0, width, height);
  }
};
