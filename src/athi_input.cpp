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
// LIABILI OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.


#include "athi_input.h"

#include "./Utility/athi_constant_globals.h"
#include "./Renderer/athi_primitives.h" // draw_hollow_circle, draw_line
#include "athi_utility.h"  // profile

#include "athi_window.h" // open_profiler
#include "./Renderer/athi_camera.h" // camera
#include "athi_particle.h"  // particle_system

#include "athi_settings.h"
#include "Utility/console.h" // console
#include "./Utility/athi_config_parser.h"  // save_variables
#include "./Renderer/athi_renderer.h"  // render_call

// ImGui::GetIO
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include <vector> // std::vector
using std::vector;

static s32 last_key;
static s32 last_action;

Athi_Input_Manager athi_input_manager;

void init_input_manager()
{
  athi_input_manager.init();
}

vec2 get_mouse_pos() noexcept
{
  const auto mouse_pos_x = athi_input_manager.mouse.pos.x;
  const auto mouse_pos_y = athi_input_manager.mouse.pos.y;
  return {mouse_pos_x, mouse_pos_y};
}

vec2 get_mouse_pos_viewspace() noexcept
{
  auto mouse_pos_x = athi_input_manager.mouse.pos.x;
  auto mouse_pos_y = athi_input_manager.mouse.pos.y;
  mouse_pos_x = -1.0f + 2 * mouse_pos_x / screen_width;
  mouse_pos_y = 1.0f - 2 * mouse_pos_y / screen_height;

  return px_scale * vec2(mouse_pos_x, mouse_pos_y);
}

vec2 get_mouse_viewspace_pos()
{
  auto context = glfwGetCurrentContext();
  f64 mouse_pos_x, mouse_pos_y;
  glfwGetCursorPos(context, &mouse_pos_x, &mouse_pos_y);

  s32 width, height;
  glfwGetWindowSize(context, &width, &height);
  mouse_pos_x = -1.0f + 2 * mouse_pos_x / width;
  mouse_pos_y = 1.0f - 2 * mouse_pos_y / height;

  return px_scale * vec2(mouse_pos_x, mouse_pos_y);
}

s32 get_mouse_button_state(s32 button) {
  const s32 state = glfwGetMouseButton(glfwGetCurrentContext(), button);
  if (state == GLFW_PRESS)
    return GLFW_PRESS;
  return GLFW_RELEASE;
}


static s32 last_mouse_button_state = GLFW_RELEASE;
s32 get_mouse_button_pressed(s32 button) {
  const s32 state = glfwGetMouseButton(glfwGetCurrentContext(), button);
  if (last_mouse_button_state == GLFW_RELEASE && state == GLFW_PRESS)
  {
    last_mouse_button_state = state;
    return true;
  }
  last_mouse_button_state = state;
  return false;
}

static void gravity_well(Particle &a, const vec2 &point) {
  const f32 x1 = a.pos.x;
  const f32 y1 = a.pos.y;
  const f32 x2 = point.x;
  const f32 y2 = point.y;
  const f32 m1 = a.mass;
  const f32 m2 = 1e11f;

  const f32 dx = x2 - x1;
  const f32 dy = y2 - y1;
  const f32 d = sqrt(dx * dx + dy * dy);

  const f32 angle = atan2(dy, dx);
  const f64 G = kGravitationalConstant;
  const f32 F = G * m1 * m2 / d * d;

  a.acc.x += F * cos(angle);
  a.acc.y += F * sin(angle);
}

void attraction_force(Particle &a, const vec2 &point) {
  // Set up variables
  const f32 x1 = a.pos.x;
  const f32 y1 = a.pos.y;
  const f32 x2 = point.x;
  const f32 y2 = point.y;

  // Get distance between balls.
  const f32 dx = x2 - x1;
  const f32 dy = y2 - y1;
  const f32 d = sqrt(dx * dx + dy * dy);

  const f32 angle = atan2(dy, dx);
  a.acc.x += d * cos(angle);
  a.acc.y += d * sin(angle);
  a.acc *= 0.7f;
}

s32 mouse_attached_to_single{-1};
enum { ATTACHED, PRESSED, NOTHING };
bool mouse_pressed{false};
u16 last_state{NOTHING};

s32 id1, id2;
bool found{false};
bool attach{false};
static bool is_dragging{false};
static std::vector<s32> mouse_attached_to;

void drag_color_or_destroy_with_mouse()
{
  // Get the mouse state
  const auto state = get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT);

  // ...and position
  const auto mouse_pos = athi_input_manager.mouse.pos;

  // Setup a rect and draw to screen if needed
  Athi_Rect mouse_rect;
  mouse_rect.min = mouse_pos - mouse_size;
  mouse_rect.max = mouse_pos + mouse_size;

  // If the mouse left button is released, just exit the function.
  if (state == GLFW_RELEASE) {
    is_dragging = false;
    last_state = NOTHING;
    mouse_attached_to.clear();
    return;
  }

  // If pressed continue on..

  if (last_state == ATTACHED) {
    is_dragging = true;
  }

  Particle p;
  p.pos = mouse_pos;
  p.radius = mouse_size;
  std::vector<s32> particle_ids_in_circle;

  // Dont get all the ids if we're doing GravityWell
  if (mouse_option != MouseOption::GravityWell) {
    particle_ids_in_circle = particle_system.get_particles_in_circle(p);
  }

  switch (mouse_option) {
    case MouseOption::Color: {
        particle_system.set_particles_color(particle_ids_in_circle, circle_color);
    } break;

    case MouseOption::GravityWell: {
      // Pull the particles towards the mouse
      particle_system.pull_towards_point(mouse_pos);
      last_state = ATTACHED;
    } break;

    case MouseOption::Drag: {
      // Add all selected particles to our list of attached particles. If not
      // already attached
      if (!is_dragging) {
        for (const auto particle_id : particle_ids_in_circle) {
          mouse_attached_to.emplace_back(particle_id);
        }
      }

      // Pull the particles towards the mouse
      for (const auto particle_id : mouse_attached_to) {
        attraction_force(particle_system.particles[particle_id], mouse_pos);

        last_state = ATTACHED;

        // Debug lines from particle to mouse
        if (show_mouse_grab_lines) {
          draw_line(mouse_pos, particle_system.particles[particle_id].pos, 1.0f, vec4(pastel_pink.x, pastel_pink.y, pastel_pink.z, 0.1));
        }
      }
    } break;
    case MouseOption::Delete: {
      // if (!selected_particle_ids.empty()) { // DOESNT WORK
      // particle_system.remove_all_with_id(selected_particle_ids);
      //}
    } break;

    case MouseOption::None: {
      /* Do nothing */
    } break;
  }
}

void update_inputs() {

  auto mouse_pos = athi_input_manager.mouse.pos;
  auto context = glfwGetCurrentContext();
  {

    drag_color_or_destroy_with_mouse();
  }

  // console->info("mouse_pos: {} : {}", mouse_pos.x, mouse_pos.y);
    if (glfwGetKey(context, GLFW_KEY_1) == GLFW_PRESS) {
     particle_system.add(mouse_pos, 1.0f, circle_color);
     particle_system.add(mouse_pos, 1.0f, circle_color);
     particle_system.add(mouse_pos, 1.0f, circle_color);
     particle_system.add(mouse_pos, 1.0f, circle_color);
     particle_system.add(mouse_pos, 1.0f, circle_color);
     particle_system.add(mouse_pos, 1.0f, circle_color);
     particle_system.add(mouse_pos, 1.0f, circle_color);
     particle_system.add(mouse_pos, 1.0f, circle_color);
     particle_system.add(mouse_pos, 1.0f, circle_color);
     particle_system.add(mouse_pos, 1.0f, circle_color);
  }

  if (glfwGetKey(context, GLFW_KEY_2) == GLFW_PRESS) {
     particle_system.add(mouse_pos, 2.5f, circle_color);
     particle_system.add(mouse_pos, 2.5f, circle_color);
     particle_system.add(mouse_pos, 2.5f, circle_color);
     particle_system.add(mouse_pos, 2.5f, circle_color);
     particle_system.add(mouse_pos, 2.5f, circle_color);
     particle_system.add(mouse_pos, 2.5f, circle_color);
     particle_system.add(mouse_pos, 2.5f, circle_color);
     particle_system.add(mouse_pos, 2.5f, circle_color);
     particle_system.add(mouse_pos, 2.5f, circle_color);
     particle_system.add(mouse_pos, 2.5f, circle_color);
  }

  if (glfwGetKey(context, GLFW_KEY_3) == GLFW_PRESS) {
     particle_system.add(mouse_pos, 5.0f, circle_color);
     particle_system.add(mouse_pos, 5.0f, circle_color);
     particle_system.add(mouse_pos, 5.0f, circle_color);
     particle_system.add(mouse_pos, 5.0f, circle_color);
     particle_system.add(mouse_pos, 5.0f, circle_color);
     particle_system.add(mouse_pos, 5.0f, circle_color);
     particle_system.add(mouse_pos, 5.0f, circle_color);
     particle_system.add(mouse_pos, 5.0f, circle_color);
     particle_system.add(mouse_pos, 5.0f, circle_color);
     particle_system.add(mouse_pos, 5.0f, circle_color);
  }

  if (glfwGetKey(context, GLFW_KEY_4) == GLFW_PRESS) {
     particle_system.add(mouse_pos, 10.0f, circle_color);
     particle_system.add(mouse_pos, 10.0f, circle_color);
     particle_system.add(mouse_pos, 10.0f, circle_color);
     particle_system.add(mouse_pos, 10.0f, circle_color);
     particle_system.add(mouse_pos, 10.0f, circle_color);
     particle_system.add(mouse_pos, 10.0f, circle_color);
     particle_system.add(mouse_pos, 10.0f, circle_color);
     particle_system.add(mouse_pos, 10.0f, circle_color);
     particle_system.add(mouse_pos, 10.0f, circle_color);
     particle_system.add(mouse_pos, 10.0f, circle_color);
  }

  if (glfwGetKey(context, GLFW_KEY_5) == GLFW_PRESS) {
     particle_system.add(mouse_pos, mouse_size, circle_color);
     particle_system.add(mouse_pos, mouse_size, circle_color);
     particle_system.add(mouse_pos, mouse_size, circle_color);
     particle_system.add(mouse_pos, mouse_size, circle_color);
     particle_system.add(mouse_pos, mouse_size, circle_color);
     particle_system.add(mouse_pos, mouse_size, circle_color);
     particle_system.add(mouse_pos, mouse_size, circle_color);
     particle_system.add(mouse_pos, mouse_size, circle_color);
     particle_system.add(mouse_pos, mouse_size, circle_color);
     particle_system.add(mouse_pos, mouse_size, circle_color);
  }

  // Draw Mouse
  Athi_Rect mouse_rect;
  mouse_rect.min = mouse_pos - mouse_size;
  mouse_rect.max = mouse_pos + mouse_size;

  draw_circle(mouse_pos, mouse_size, circle_color, true);
  if (draw_debug && show_mouse_collision_box) {
      draw_rect(mouse_rect.min, mouse_rect.max, pastel_green, true);
  }
}

void scroll_callback(GLFWwindow *window, f64 xoffset, f64 yoffset)
{
  mouse_size -= yoffset * 0.5f;
  if (mouse_size < 0.000f) mouse_size = 0.5f;
  g_MouseWheel +=
      (f32)yoffset;  // Use fractional mouse wheel, 1.0 unit 5 lines.

  camera.process_mouse_scroll(yoffset);
  camera.update();
}
void cursor_position_callback(GLFWwindow *window, f64 xpos, f64 ypos)
{
  athi_input_manager.mouse.pos.x = px_scale *  xpos;
  athi_input_manager.mouse.pos.y = framebuffer_height - (px_scale *  ypos);
}

void char_callback(GLFWwindow *, u32 c)
{
  ImGuiIO &io = ImGui::GetIO();
  if (c > 0 && c < 0x10000) io.AddInputCharacter((u16)c);
}

void mouse_button_callback(GLFWwindow *window, s32 button, s32 action, s32 mods)
{
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

  vec2 mouse_pos = athi_input_manager.mouse.pos;

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    particle_system.add(mouse_pos, mouse_size, circle_color);
  }
}

void key_callback(GLFWwindow *window, s32 key, s32 scancode, s32 action, s32 mods)
{
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
    if (!quadtree_active) {
      quadtree_active = true;
      use_uniformgrid = false;
    }
    else {
      quadtree_active = false;
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
    if (!use_uniformgrid) {
      use_uniformgrid = true;
      quadtree_active = false;
    }
    else {
      use_uniformgrid = false;
    }
    console->info("UniformGrid: {}", use_uniformgrid ? "ON" : "OFF");
  }

  if (quadtree_active) tree_type = TreeType::Quadtree;
  else if (use_uniformgrid) tree_type = TreeType::UniformGrid;
  else tree_type = TreeType::None;


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
        particle_system.add(vec2(j, k), 1.0f, circle_color);

    for (s32 j = framebuffer_width - 250; j > framebuffer_width-500; j -= 2)
      for (s32 k = framebuffer_height - 250; k > framebuffer_height-500; k -= 2)
        particle_system.add(vec2(j, k), 1.0f, circle_color);
  }

  // Benchmark 2
  if (key_pressed(GLFW_KEY_N)) {
    for (s32 j = 0; j < framebuffer_width; j += 4)
      for (s32 k = 0; k < framebuffer_height; k += 4)
        particle_system.add(vec2(j, k), 1.0f, circle_color);
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
