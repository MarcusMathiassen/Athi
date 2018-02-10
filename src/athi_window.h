// Copyright (c) 2018 Marcus Mathiassen

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.


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
  static void window_pos_callback(GLFWwindow* window, int xpos, int ypos);

  string title{"Athi"};
  struct {
    u32 width{512}, height{512};
  } scene, view;

  void init();

  GLFWwindow *get_window_context();
};
