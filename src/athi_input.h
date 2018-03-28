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

#include "Utility/fixed_size_types.h" // u32, s32, etc.

// GLFWwindow
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp> // glm::vec2;
using glm::vec2;

// Returns the mouse position inside the window.
vec2 get_mouse_pos() noexcept;
// Returns the mouse position in viewspace.
vec2 get_mouse_pos_viewspace() noexcept;

void init_input_manager();
vec2 get_mouse_viewspace_pos();
s32 get_mouse_button_state(s32 button);
s32 get_mouse_button_pressed(s32 button);
void update_inputs();
void scroll_callback(GLFWwindow *window, f64 xoffset, f64 yoffset);
void cursor_position_callback(GLFWwindow *window, f64 xpos, f64 ypos);
void char_callback(GLFWwindow *, u32 c);
void mouse_button_callback(GLFWwindow *window, s32 button, s32 action, s32 mods);
void key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods);

struct Mouse
{
  vec2 pos;
  struct Button
  {
    bool state;
  } left_button, right_button;
};

struct Athi_Input_Manager
{
  Mouse mouse;
  void init()
  {
    auto context = glfwGetCurrentContext();
    glfwSetMouseButtonCallback(context, mouse_button_callback);
    glfwSetKeyCallback(context, key_callback);
    glfwSetCharCallback(context, char_callback);
    glfwSetCursorPosCallback(context, cursor_position_callback);
    glfwSetScrollCallback(context, scroll_callback);
  }
};

extern Athi_Input_Manager athi_input_manager;
