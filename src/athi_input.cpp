
#include "athi_typedefs.h"
#include "athi_input.h"
#include "athi_line.h"
#include "athi_particle.h"
#include "athi_rect.h"
#include "athi_renderer.h"
#include "athi_settings.h"
#include "athi_spring.h"
#include "athi_utility.h"

#include <glm/vec2.hpp>
#include <iostream>

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

  return vec2(mouse_pos_x, mouse_pos_y);
}

s32 get_mouse_button_state(s32 button) {
  const s32 state = glfwGetMouseButton(glfwGetCurrentContext(), button);
  if (state == GLFW_PRESS)
    return GLFW_PRESS;
  return GLFW_RELEASE;
}

static void gravity_well(Particle &a, const vec2 &point) {
  const f32 x1 = a.pos.x;
  const f32 y1 = a.pos.y;
  const f32 x2 = point.x;
  const f32 y2 = point.y;
  const f32 m1 = a.mass;
  const f32 m2 = 100000.0f;

  const f32 dx = x2 - x1;
  const f32 dy = y2 - y1;
  const f32 d = sqrt(dx * dx + dy * dy);

  const f32 angle = atan2(dy, dx);
  const f32 G = gravitational_constant;
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

int32_t mouse_attached_to_single{-1};
enum { ATTACHED, PRESSED, NOTHING };
bool mouse_pressed{false};
uint16_t last_state{NOTHING};

int32_t id1, id2;
bool found{false};
bool attach{false};
static bool is_dragging{false};
static vector<s32> mouse_attached_to;
void drag_color_or_destroy_with_mouse() {


  // Get the mouse state
  const auto state = get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT);

  // ...and position
  const auto mouse_pos = athi_input_manager.mouse.pos;

  // Setup a rect and draw to screen if needed
  const Athi::Rect mouse_rect(mouse_pos - mouse_size, mouse_pos + mouse_size);
  if (draw_debug && show_mouse_collision_box) {
    draw_hollow_rect(mouse_rect.min, mouse_rect.max, pastel_green);
  }

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

  // Get all particles in the mouse collision box
  const auto selected_particle_ids = get_particles_in_rect(particle_system.particles, mouse_rect.min, mouse_rect.max);

  switch (mouse_option) {

    case MouseOption::Color: { 
      for (const auto particle_id: selected_particle_ids) {
        particle_system.colors[particle_id] = circle_color; 
      } 
    } break;

    case MouseOption::GravityWell: {
      // Pull the particles towards the mouse
      for (auto& particle: particle_system.particles) {
        gravity_well(particle, mouse_pos);
        last_state = ATTACHED;
      }
    } break;

    case MouseOption::Drag: { 

      // Add all selected particles to our list of attached particles. If not already attached
      if (!is_dragging) {
        for (const auto particle_id: selected_particle_ids) {
          mouse_attached_to.emplace_back(particle_id);
        }
      }

      // Pull the particles towards the mouse
      for (const auto particle_id: mouse_attached_to) {
        attraction_force(particle_system.particles[particle_id], mouse_pos);

        last_state = ATTACHED;

        // Debug lines from particle to mouse
        if (draw_debug && show_mouse_grab_lines) {
          auto ms_view_pos = to_view_space(mouse_pos);
          auto p_view_pos = to_view_space(particle_system.particles[particle_id].pos);
          draw_line(ms_view_pos, p_view_pos, 1.0f, pastel_pink);
        }
      }
    } break;
    case MouseOption::Delete: {
      //if (!selected_particle_ids.empty()) { // DOESNT WORK
        //particle_system.remove_all_with_id(selected_particle_ids);
      //}
    } break;

    case MouseOption::None: {
      /* Do nothing */
    } break;
  }
}

void update_inputs() {
  profile p("update_inputs");

  auto mouse_pos = athi_input_manager.mouse.pos;
  auto context = glfwGetCurrentContext();
  {
    profile p("drag_color_or_destroy_with_mouse");
    drag_color_or_destroy_with_mouse();
  }

  // Draw Mouse
  const Athi::Rect mouse_rect(mouse_pos - mouse_size, mouse_pos + mouse_size);
  draw_hollow_rect(mouse_rect.min, mouse_rect.max, circle_color);


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
    particle_system.add(mouse_pos, 2.0f, circle_color);
    particle_system.add(mouse_pos, 2.0f, circle_color);
    particle_system.add(mouse_pos, 2.0f, circle_color);
    particle_system.add(mouse_pos, 2.0f, circle_color);
    particle_system.add(mouse_pos, 2.0f, circle_color);
    particle_system.add(mouse_pos, 2.0f, circle_color);
    particle_system.add(mouse_pos, 2.0f, circle_color);
    particle_system.add(mouse_pos, 2.0f, circle_color);
    particle_system.add(mouse_pos, 2.0f, circle_color);
    particle_system.add(mouse_pos, 2.0f, circle_color);
  }

  if (glfwGetKey(context, GLFW_KEY_3) == GLFW_PRESS) {
    particle_system.add(mouse_pos, 3.0f, circle_color);
    particle_system.add(mouse_pos, 3.0f, circle_color);
    particle_system.add(mouse_pos, 3.0f, circle_color);
    particle_system.add(mouse_pos, 3.0f, circle_color);
    particle_system.add(mouse_pos, 3.0f, circle_color);
    particle_system.add(mouse_pos, 3.0f, circle_color);
    particle_system.add(mouse_pos, 3.0f, circle_color);
    particle_system.add(mouse_pos, 3.0f, circle_color);
    particle_system.add(mouse_pos, 3.0f, circle_color);
    particle_system.add(mouse_pos, 3.0f, circle_color);
  }

  if (glfwGetKey(context, GLFW_KEY_4) == GLFW_PRESS) {
    particle_system.add(mouse_pos, 4.0f, circle_color);
    particle_system.add(mouse_pos, 4.0f, circle_color);
    particle_system.add(mouse_pos, 4.0f, circle_color);
    particle_system.add(mouse_pos, 4.0f, circle_color);
    particle_system.add(mouse_pos, 4.0f, circle_color);
    particle_system.add(mouse_pos, 4.0f, circle_color);
    particle_system.add(mouse_pos, 4.0f, circle_color);
    particle_system.add(mouse_pos, 4.0f, circle_color);
    particle_system.add(mouse_pos, 4.0f, circle_color);
    particle_system.add(mouse_pos, 4.0f, circle_color);
  }
  if (glfwGetKey(context, GLFW_KEY_5) == GLFW_PRESS) {
    particle_system.add(mouse_pos, circle_size, circle_color);
    particle_system.add(mouse_pos, circle_size, circle_color);
    particle_system.add(mouse_pos, circle_size, circle_color);
    particle_system.add(mouse_pos, circle_size, circle_color);
    particle_system.add(mouse_pos, circle_size, circle_color);
    particle_system.add(mouse_pos, circle_size, circle_color);
    particle_system.add(mouse_pos, circle_size, circle_color);
    particle_system.add(mouse_pos, circle_size, circle_color);
    particle_system.add(mouse_pos, circle_size, circle_color);
    particle_system.add(mouse_pos, circle_size, circle_color);
  }
}
