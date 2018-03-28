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

#include "athi_window.h"    // open_profiler
#include "./Renderer/athi_camera.h"    // camera
#include "athi_particle.h"  // particle_system
#include "athi_settings.h"
#include "Utility/console.h" // console
#include "./Utility/athi_config_parser.h"  // save_variables
#include "./Renderer/athi_renderer.h"  // render_call

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include <deque> // std::dequeue

#include <GL/glew.h>
#include <GLFW/glfw3.h>

static f32 hue = 0;
static bool left_shift_pressed = false;

static s32 last_key;
static s32 last_action;

enum class Action
{
  PRESSED = GLFW_PRESS,
  RELEASED = GLFW_RELEASE,
};

enum class Key
{
  UNKNOWN = GLFW_KEY_UNKNOWN,
  SPACE = GLFW_KEY_SPACE,
  APOSTROPHE = GLFW_KEY_APOSTROPHE,
  COMMA = GLFW_KEY_COMMA,
  MINUS = GLFW_KEY_MINUS,
  PERIOD = GLFW_KEY_PERIOD,
  SLASH = GLFW_KEY_SLASH,
  NUM_0 = GLFW_KEY_0,
  NUM_1 = GLFW_KEY_1,
  NUM_2 = GLFW_KEY_2,
  NUM_3 = GLFW_KEY_3,
  NUM_4 = GLFW_KEY_4,
  NUM_5 = GLFW_KEY_5,
  NUM_6 = GLFW_KEY_6,
  NUM_7 = GLFW_KEY_7,
  NUM_8 = GLFW_KEY_8,
  NUM_9 = GLFW_KEY_9,
  SEMICOLON = GLFW_KEY_SEMICOLON,
  EQUAL = GLFW_KEY_EQUAL,
  A = GLFW_KEY_A,
  B = GLFW_KEY_B,
  C = GLFW_KEY_C,
  D = GLFW_KEY_D,
  E = GLFW_KEY_E,
  F = GLFW_KEY_F,
  G = GLFW_KEY_G,
  H = GLFW_KEY_H,
  I = GLFW_KEY_I,
  J = GLFW_KEY_J,
  K = GLFW_KEY_K,
  L = GLFW_KEY_L,
  M = GLFW_KEY_M,
  N = GLFW_KEY_N,
  O = GLFW_KEY_O,
  P = GLFW_KEY_P,
  Q = GLFW_KEY_Q,
  R = GLFW_KEY_R,
  S = GLFW_KEY_S,
  T = GLFW_KEY_T,
  U = GLFW_KEY_U,
  V = GLFW_KEY_V,
  W = GLFW_KEY_W,
  X = GLFW_KEY_X,
  Y = GLFW_KEY_Y,
  Z = GLFW_KEY_Z,
  LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET,
  BACKSLASH = GLFW_KEY_BACKSLASH,
  RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET,
  GRAVE_ACCENT = GLFW_KEY_GRAVE_ACCENT,
  WORLD_1 = GLFW_KEY_WORLD_1,
  WORLD_2 = GLFW_KEY_WORLD_2,
  ESCAPE = GLFW_KEY_ESCAPE,
  ENTER = GLFW_KEY_ENTER,
  TAB = GLFW_KEY_TAB,
  BACKSPACE = GLFW_KEY_BACKSPACE,
  INSERT = GLFW_KEY_INSERT,
  DELETE = GLFW_KEY_DELETE,
  RIGHT = GLFW_KEY_RIGHT,
  LEFT = GLFW_KEY_LEFT,
  DOWN = GLFW_KEY_DOWN,
  UP = GLFW_KEY_UP,
  PAGE_UP = GLFW_KEY_PAGE_UP,
  PAGE_DOWN = GLFW_KEY_PAGE_DOWN,
  HOME = GLFW_KEY_HOME,
  END = GLFW_KEY_END,
  CAPS_LOCK = GLFW_KEY_CAPS_LOCK,
  SCROLL_LOCK = GLFW_KEY_SCROLL_LOCK,
  NUM_LOCK = GLFW_KEY_NUM_LOCK,
  PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN,
  PAUSE = GLFW_KEY_PAUSE,
  F1 = GLFW_KEY_F1,
  F2 = GLFW_KEY_F2,
  F3 = GLFW_KEY_F3,
  F4 = GLFW_KEY_F4,
  F5 = GLFW_KEY_F5,
  F6 = GLFW_KEY_F6,
  F7 = GLFW_KEY_F7,
  F8 = GLFW_KEY_F8,
  F9 = GLFW_KEY_F9,
  F10 = GLFW_KEY_F10,
  F11 = GLFW_KEY_F11,
  F12 = GLFW_KEY_F12,
  F13 = GLFW_KEY_F13,
  F14 = GLFW_KEY_F14,
  F15 = GLFW_KEY_F15,
  F16 = GLFW_KEY_F16,
  F17 = GLFW_KEY_F17,
  F18 = GLFW_KEY_F18,
  F19 = GLFW_KEY_F19,
  F20 = GLFW_KEY_F20,
  F21 = GLFW_KEY_F21,
  F22 = GLFW_KEY_F22,
  F23 = GLFW_KEY_F23,
  F24 = GLFW_KEY_F24,
  F25 = GLFW_KEY_F25,
  KP_0 = GLFW_KEY_KP_0,
  KP_1 = GLFW_KEY_KP_1,
  KP_2 = GLFW_KEY_KP_2,
  KP_3 = GLFW_KEY_KP_3,
  KP_4 = GLFW_KEY_KP_4,
  KP_5 = GLFW_KEY_KP_5,
  KP_6 = GLFW_KEY_KP_6,
  KP_7 = GLFW_KEY_KP_7,
  KP_8 = GLFW_KEY_KP_8,
  KP_9 = GLFW_KEY_KP_9,
  KP_DECIMAL = GLFW_KEY_KP_DECIMAL,
  KP_DIVIDE = GLFW_KEY_KP_DIVIDE,
  KP_MULTIPLY = GLFW_KEY_KP_MULTIPLY,
  KP_SUBTRACT = GLFW_KEY_KP_SUBTRACT,
  KP_ADD = GLFW_KEY_KP_ADD,
  KP_ENTER = GLFW_KEY_KP_ENTER,
  KP_EQUAL = GLFW_KEY_KP_EQUAL,
  LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT,
  LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL,
  LEFT_ALT = GLFW_KEY_LEFT_ALT,
  LEFT_SUPER = GLFW_KEY_LEFT_SUPER,
  RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT,
  RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL,
  RIGHT_ALT = GLFW_KEY_RIGHT_ALT,
  RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER,
};

// Checks if keys are pressed
// static bool key_down(Key key, Action action)
// {
//   if (glfwGetKey(glfwGetCurrentContext(), (int)key) == action)
//     return true;
//   return false;
// }


void init_input_manager();
glm::vec2 get_mouse_viewspace_pos();
int32_t get_mouse_button_state(int32_t button);
int32_t get_mouse_button_pressed(int32_t button);

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

  mouse_size -= yoffset * 0.5f;
  if (mouse_size < 0.000f) mouse_size = 0.5f;
  g_MouseWheel +=
      (f32)yoffset;  // Use fractional mouse wheel, 1.0 unit 5 lines.

  // if (last_key == GLFW_KEY_LEFT_CONTROL && last_action == GLFW_PRESS)
  // {
      camera.process_mouse_scroll(yoffset);
      camera.update();
  // }
}

static void cursor_position_callback(GLFWwindow *window, f64 xpos, f64 ypos) {
  athi_input_manager.mouse.pos.x = px_scale *  xpos;
  athi_input_manager.mouse.pos.y = framebuffer_height - (px_scale *  ypos);
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
                         s32 mods)
{
  auto mouse_pos = athi_input_manager.mouse.pos;

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

  const auto key_pressed = [key, action](s32 key_code)
  {
    return key == key_code && action == GLFW_PRESS;
  };

  // Save all state
  if ( (last_key == GLFW_KEY_LEFT_SUPER || last_key == GLFW_KEY_LEFT_CONTROL) && last_action == GLFW_PRESS && key_pressed(GLFW_KEY_S))
  {
    // Not yet threadsafe
    save_variables();
    particle_system.save_state();
    console->warn("config saved!");
  }

  // load state
  if (
      (last_key == GLFW_KEY_LEFT_SUPER || last_key == GLFW_KEY_LEFT_CONTROL) && last_action == GLFW_PRESS &&
      key_pressed(GLFW_KEY_X))
  {
    particle_system.load_state();
  }

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
    if (gravity > 0) gravity = 0.0f;
    else gravity = 0.981f;
    console->info("Particle gravity: {}", gravity > 0 ? "ON" : "OFF");
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
    if (tree_radio_option != (s32)TreeType::Quadtree)
      tree_radio_option = (s32)TreeType::Quadtree;
    else tree_radio_option = (s32)TreeType::None;
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

  if (quadtree_active) tree_type = TreeType::Quadtree;
  else if (use_uniformgrid) tree_type = TreeType::UniformGrid;
  else tree_type = TreeType::None;

  // TOGGLE UniformGrid ACTIVE
  if (key_pressed(GLFW_KEY_W)) {
    if (tree_radio_option != (s32)TreeType::UniformGrid)
      tree_radio_option = (s32)TreeType::UniformGrid;
    else tree_radio_option = (s32)TreeType::None;
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

    for (s32 j = framebuffer_width - 250; j > framebuffer_width-500; j -= 2)
      for (s32 k = framebuffer_height - 250; k > framebuffer_height-500; k -= 2)
        particle_system.add(glm::vec2(j, k), 1.0f, circle_color);
  }

  // Benchmark 2
  if (key_pressed(GLFW_KEY_N)) {
    for (s32 j = 0; j < framebuffer_width; j += 4)
      for (s32 k = 0; k < framebuffer_height; k += 4)
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

  last_key = key;
  last_action = action;
}
