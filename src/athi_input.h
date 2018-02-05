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

#include "./Renderer/athi_camera.h"    // camera
#include "athi_particle.h"  // particle_system
#include "athi_settings.h"  // console

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static f32 hue = 0;
static bool left_shift_pressed = false;

void init_input_manager();
glm::vec2 get_mouse_viewspace_pos();
int32_t get_mouse_button_state(int32_t button);
void update_inputs();
static void mouse_button_callback(GLFWwindow *window, s32 button, s32 action,
                                  s32 mods);
static void cursor_position_callback(GLFWwindow *window, f64 xpos, f64 ypos);
static void key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action,
                         s32 mods);
static void scroll_callback(GLFWwindow *window, f64 xoffset, f64 yoffset);
static void char_callback(GLFWwindow *, u32 c);
struct Mouse {
  glm::vec2 pos;
  struct Button {
    bool state;
  } left_button, right_button;
};

struct Athi_Input_Manager {
  Mouse mouse;
  void init() {
    auto context = glfwGetCurrentContext();
    glfwSetMouseButtonCallback(context, mouse_button_callback);
    glfwSetKeyCallback(context, key_callback);
    glfwSetCharCallback(context, char_callback);
    glfwSetCursorPosCallback(context, cursor_position_callback);
    glfwSetScrollCallback(context, scroll_callback);
  }
};

extern Athi_Input_Manager athi_input_manager;

static void scroll_callback(GLFWwindow *window, f64 xoffset, f64 yoffset) {
  // If color wheel
  //if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    hue += xoffset;
    circle_color = hsv_to_rgb(hue, 1.0, 1.0, 1.0);
    //return;
  //}

  mouse_size -= yoffset * 0.5f;
  if (mouse_size < 0.000f) mouse_size = 0.5f;
  g_MouseWheel +=
      (f32)yoffset;  // Use fractional mouse wheel, 1.0 unit 5 lines.

  // camera.process_mouse_scroll(yoffset);
  // camera.update();
}

static void cursor_position_callback(GLFWwindow *window, f64 xpos, f64 ypos) {
  athi_input_manager.mouse.pos.x = xpos * px_scale;
  athi_input_manager.mouse.pos.y = screen_height - (ypos * px_scale);
}

static void char_callback(GLFWwindow *, u32 c) {
  ImGuiIO &io = ImGui::GetIO();
  if (c > 0 && c < 0x10000) io.AddInputCharacter((u16)c);
}

static void mouse_button_callback(GLFWwindow *window, s32 button, s32 action,
                                  s32 mods) {
  if (action == GLFW_PRESS && button >= 0 && button < 3)
    g_MouseJustPressed[button] = true;

  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      athi_input_manager.mouse.left_button.state = action;
      break;
    case GLFW_MOUSE_BUTTON_RIGHT:
      athi_input_manager.mouse.right_button.state = action;
      break;
  }

  glm::vec2 mouse_pos = athi_input_manager.mouse.pos;

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    particle_system.add(mouse_pos, mouse_size, circle_color);
  }
}

static void key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action,
                         s32 mods) {
  // IMGUI
  ImGuiIO &io = ImGui::GetIO();
  if (action == GLFW_PRESS) io.KeysDown[key] = true;
  if (action == GLFW_RELEASE) io.KeysDown[key] = false;

  (void)mods;  // Modifiers are not reliable across systems
  io.KeyCtrl =
      io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
  io.KeyShift =
      io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
  io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
  io.KeySuper =
      io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

  //

  const auto key_pressed = [key, action](s32 key_code)
  {
    return key == key_code && action == GLFW_PRESS;
  };

  // TOGGLE PAUSE
  if (key_pressed(GLFW_KEY_SPACE)) {
    if (time_scale == 0)
      time_scale = 1;
    else
      time_scale = 0;

    console->info("Paused: {}", time_scale ? "ON" : "OFF");
  }

  // TOGGLE SETTINGS UI
  if (key_pressed(GLFW_KEY_I)) {
    show_settings ^= 1;
    console->info("Show menu: {}", show_settings ? "ON" : "OFF");
  }

  // TOGGLE VSYNC
  if (key_pressed(GLFW_KEY_L)) {
    vsync ^= 1;
    console->info("Vsync: {}", vsync ? "ON" : "OFF");
  }

  // TOGGLE DEBUG UI
  if (key_pressed(GLFW_KEY_D)) {
    draw_debug ^= 1;
    console->info("Drag debug info: {}", draw_debug ? "ON" : "OFF");
  }

  // TOGGLE CIRCLE GRAVITY
  if (key_pressed(GLFW_KEY_G)) {
    physics_gravity ^= 1;
    console->info("Particle gravity: {}", physics_gravity ? "ON" : "OFF");
  }

  // TOGGLE CIRCLE COLLISIONS
  if (key_pressed(GLFW_KEY_C)) {
    circle_collision ^= 1;
    console->info("Particle intercollisions: {}",
                  circle_collision ? "ON" : "OFF");
  }

  // TOGGLE GRAVITATIONAL_FORCES
  if (key_pressed(GLFW_KEY_T)) {
    use_gravitational_force ^= 1;
  }

  // TOGGLE MULTITHREADING
  if (key_pressed(GLFW_KEY_M)) {
    use_multithreading ^= 1;
    console->info("Multithreading: {}", use_multithreading ? "ON" : "OFF");
  }

  // TOGGLE OPENCL
  if (key_pressed(GLFW_KEY_A)) {
    openCL_active ^= 1;
    console->info("OpenCL: {}", openCL_active ? "ON" : "OFF");
  }

  // TOGGLE MOUSE GRAB LINES
  if (key_pressed(GLFW_KEY_Y)) {
    show_mouse_grab_lines ^= 1;
    console->info("Show mousegrab lines: {}",
                  show_mouse_grab_lines ? "ON" : "OFF");
  }

  // TOGGLE POST PROCESSING
  if (key_pressed(GLFW_KEY_P)) {
    post_processing ^= 1;
    console->info("Post processing: {}", post_processing ? "ON" : "OFF");
  }

  // TOGGLE QUADTREE ACTIVE
  if (key_pressed(GLFW_KEY_Q)) {
    if (quadtree_active) {
      quadtree_active = false;
      tree_type = TreeType::None;
    } else {
      tree_type = TreeType::Quadtree;
      quadtree_active = true;
      use_uniformgrid = false;
    }
    console->info("Quadtree: {}", quadtree_active ? "ON" : "OFF");
  }

  // TOGGLE WIREFRAME
  if (key_pressed(GLFW_KEY_GRAVE_ACCENT)) {
    if (wireframe_mode) {
      wireframe_mode = false;
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      wireframe_mode = true;
    }
    console->info("Wireframe mode: {}", wireframe_mode ? "ON" : "OFF");
  }

  // TOGGLE UniformGrid ACTIVE
  if (key_pressed(GLFW_KEY_W)) {
    if (use_uniformgrid) {
      use_uniformgrid = false;
      tree_type = TreeType::None;
    } else {
      tree_type = TreeType::UniformGrid;
      use_uniformgrid = true;
      quadtree_active = false;
    }
    console->info("UniformGrid: {}", use_uniformgrid ? "ON" : "OFF");
  }

  if (key_pressed(GLFW_KEY_6) && uniformgrid_parts > 4) {
    uniformgrid_parts *= 0.25f;
  }

  if (key_pressed(GLFW_KEY_7) && uniformgrid_parts < 512) {
    uniformgrid_parts *= 4;
  }

  // Benchmark 1
  if (key_pressed(GLFW_KEY_B)) {
    for (s32 j = 250; j < 500; j += 2)
      for (s32 k = 250; k < 500; k += 2)
        particle_system.add(glm::vec2(j, k), 1.0f, circle_color);
  }

  // Benchmark 2
  if (key_pressed(GLFW_KEY_N)) {
    for (s32 j = 0; j < screen_width; j += 4)
      for (s32 k = 0; k < screen_height; k += 4)
        particle_system.add(glm::vec2(j, k), 1.0f, circle_color);
  }

  // ERASE ALL CIRCLES
  if (key_pressed(GLFW_KEY_E)) {
    particle_system.erase_all();
  }

  if (key_pressed(GLFW_KEY_ESCAPE)) {
    mouse_option = MouseOption::None;
    mouse_radio_options = static_cast<s32>(MouseOption::None);
  }
}
