#pragma once

#include "athi_typedefs.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h> // GLFWwindow

class Athi_Window {
private:
  GLFWwindow *context{nullptr};

public:
  static void window_size_callback(GLFWwindow *window, s32 xpos, s32 ypos);
  static void framebuffer_size_callback(GLFWwindow *window, s32 width, s32 height);

  string title{"Athi"};
  struct {
    u32 width{512}, height{512};
  } scene, view;

  void init();

  GLFWwindow *get_window_context();
};
