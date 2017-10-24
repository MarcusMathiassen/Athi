#pragma once

#include "athi_particle.h"
#include "athi_settings.h"
#include "athi_task_manager.h"
#include "athi_typedefs.h"
#include "athi_window.h"

#include <memory>

struct Athi_Core {
  std::unique_ptr<Athi_Window> window;
  TaskManager thread_manager;

  void update();
  void draw(GLFWwindow *window);
  
  void window_loop();
  void draw_loop();
  void physics_loop();

  void init();
  void shutdown();

  void start();

  void update_settings();
};
