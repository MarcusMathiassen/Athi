#pragma once

#include "athi_window.h"

#include <memory>

struct Athi_Core {  
  
  Athi_Window window;

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
