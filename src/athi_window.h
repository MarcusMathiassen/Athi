#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "athi_typedefs.h"

class Athi_Window {
 private:
  GLFWwindow *context{nullptr};

 public:
  u16 monitor_refreshrate{60};

  string title{"Athi"};
  struct {
    u32 width{512}, height{512};
  } scene, view;

  void open();
  void init();
  void update();

  GLFWwindow *get_window_context();

};
