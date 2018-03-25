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
#include "./Utility/profiler.h" // cpu_profile, gpu_profile

#include "./Renderer/athi_primitives.h" // draw_hollow_circle, draw_line
#include "athi_utility.h"  // profile

Athi_Input_Manager athi_input_manager;

void init_input_manager() { athi_input_manager.init(); }

vec2 get_mouse_viewspace_pos() {
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
  a.vel.x += d * cos(angle);
  a.vel.y += d * sin(angle);
  a.vel *= 0.7f;
}

s32 mouse_attached_to_single{-1};
enum { ATTACHED, PRESSED, NOTHING };
bool mouse_pressed{false};
u16 last_state{NOTHING};

s32 id1, id2;
bool found{false};
bool attach{false};
static bool is_dragging{false};
static vector<s32> mouse_attached_to;

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
  vector<s32> particle_ids_in_circle;

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
  cpu_profile p("update_inputs");

  auto mouse_pos = athi_input_manager.mouse.pos;
  auto context = glfwGetCurrentContext();
  {
    cpu_profile p("drag_color_or_destroy_with_mouse");
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
