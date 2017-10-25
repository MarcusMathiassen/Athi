#include "athi_input.h"
#include "athi_particle.h"
#include "athi_line.h"
#include "athi_rect.h"
#include "athi_renderer.h"
#include "athi_spring.h"
#include "athi_settings.h"

Athi_Input_Manager athi_input_manager;

void init_input_manager() { athi_input_manager.init(); }

glm::vec2 get_mouse_viewspace_pos() {
  auto context = glfwGetCurrentContext();
  double mouse_pos_x, mouse_pos_y;
  glfwGetCursorPos(context, &mouse_pos_x, &mouse_pos_y);

  int32_t width, height;
  glfwGetWindowSize(context, &width, &height);
  mouse_pos_x = -1.0f + 2 * mouse_pos_x / width;
  mouse_pos_y = 1.0f - 2 * mouse_pos_y / height;

  return glm::vec2(mouse_pos_x, mouse_pos_y);
}

int32_t get_mouse_button_state(int32_t button) {
  const int state = glfwGetMouseButton(glfwGetCurrentContext(), button);
  if (state == GLFW_PRESS)
      return GLFW_PRESS;
  return GLFW_RELEASE;
}

void gravitational_force(Particle &a, const vec2 &point) {
  const float x1 = a.pos.x;
  const float y1 = a.pos.y;
  const float x2 = point.x;
  const float y2 = point.y;
  const float m1 = a.mass;
  const float m2 = 10.0f;

  const float dx = x2 - x1;
  const float dy = y2 - y1;
  const float d = sqrt(dx * dx + dy * dy);

  if (d > 1e-4f) {
    const float angle = atan2(dy, dx);
    const float G = 6.674f;
    const float F = G * m1 * m2 / d * d;

    a.vel.x += F * cos(angle);
    a.vel.y += F * sin(angle);
  }
}

void attraction_force(Particle &a, const vec2 &point) {
  // Set up variables
  const float x1 = a.pos.x;
  const float y1 = a.pos.y;
  const float x2 = point.x;
  const float y2 = point.y;

  // Get distance between balls.
  const float dx = x2 - x1;
  const float dy = y2 - y1;
  const float d = sqrt(dx * dx + dy * dy);

  const float angle = atan2(dy, dx);
  a.vel.x += 0.1f * d * cos(angle);
  a.vel.y += 0.1f * d * sin(angle);
  a.vel *= 0.7f;
}

vector<int32_t> mouse_attached_to;
int32_t mouse_attached_to_single{-1};
enum { ATTACHED, PRESSED, NOTHING };
uint16_t last_state{NOTHING};

int32_t id1, id2;
bool found{false};
bool attach{false};
void mouse_attach_spring() {
  // Drag a spring from one circle to the other
  // Get mouse position
  vec2 mouse_pos = get_mouse_viewspace_pos();

  Athi::Rect mouse_rect(
      vec2(mouse_pos.x - mouse_size, mouse_pos.y - mouse_size),
      vec2(mouse_pos.x + mouse_size, mouse_pos.y + mouse_size));

  int32_t state = get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT);

  if (attach) {
    attach_spring(particle_manager.particles[id1],
                  particle_manager.particles[id2]);
    attach = false;
  }

  // If mouse released and first circle is found
  if (state == GLFW_RELEASE && found) {
    for (auto &c : particle_manager.particles) {
      if (mouse_rect.contains(c.id)) {
        id2 = c.id;
        attach = true;
        found = false;
        return;
      }
    }
  }

  // draw a line while we look for the second circle
  if (state == GLFW_PRESS && found) {
    draw_line(mouse_pos, particle_manager.particles[id1].pos, 0.03f,
              pastel_pink);
    return;
  }

  // When the mouse is pressed down, get the first circle id
  if (state == GLFW_PRESS) {
    for (auto &c : particle_manager.particles) {
      if (mouse_rect.contains(c.id)) {
        id1 = c.id;
        found = true;
        return;
      }
    }
  }
}

void mouse_grab_particles() {
  // Get the mouse state
  int32_t state = get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT);

  auto mouse_pos = athi_input_manager.mouse.pos;

  Athi::Rect mouse_rect(mouse_pos - mouse_size, mouse_pos + mouse_size);
  if (draw_debug)
    draw_hollow_rect(mouse_rect.min, mouse_rect.max, pastel_green);

  // If it's released just exit the function
  if (state == GLFW_RELEASE) {
    last_state = NOTHING;
    mouse_busy_UI = false;

    // remove all circles attached
    mouse_attached_to.clear();
    return;
  }
  // If pressed continue on..

  // If already attached
  if (last_state == ATTACHED) {
    if (mouse_grab_multiple) {
      for (auto &id : mouse_attached_to) {
        attraction_force(particle_manager.particles[id], mouse_pos);
        last_state = ATTACHED;
        if (show_mouse_grab_lines)
          draw_line(mouse_pos, particle_manager.particles[id].pos,
                    0.03f, pastel_pink);
      }
    } else  // single
    {
      attraction_force(
          particle_manager.particles[mouse_attached_to_single],
          mouse_pos);
      if (show_mouse_grab_lines)
        draw_line(
            mouse_pos,
            particle_manager.particles[mouse_attached_to_single].pos,
            0.03f, pastel_pink);
    }
    mouse_busy_UI = true;
  }

  // Go through all circles. Return the circle hovered
  if (last_state != ATTACHED) {
    for (auto &c : particle_manager.particles) {
      // If the mouse and circle intersect
      if (mouse_rect.contains(c.id)) {
        if (mouse_grab_multiple) {
          mouse_attached_to.emplace_back(c.id);
        } else {
          mouse_attached_to_single = c.id;
        }
        last_state = ATTACHED;
      }
    }
  }
}

void update_inputs() {
  auto mouse_pos = athi_input_manager.mouse.pos;
  auto context = glfwGetCurrentContext();

  // Find the circle you're over
  if (glfwGetKey(context, GLFW_KEY_S) == GLFW_PRESS) {
    mouse_attach_spring();
  } 
  if (mouse_grab)
    mouse_grab_particles();

  if (glfwGetKey(context, GLFW_KEY_1) == GLFW_PRESS) {
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 1.0f, glm::vec4(1,1,1,1));
  }

  if (glfwGetKey(context, GLFW_KEY_2) == GLFW_PRESS) {
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 2.0f, glm::vec4(1,1,1,1));
  }

  if (glfwGetKey(context, GLFW_KEY_3) == GLFW_PRESS) {
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 3.0f, glm::vec4(1,1,1,1));
  }

  if (glfwGetKey(context, GLFW_KEY_4) == GLFW_PRESS) {
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
    particle_manager.add(mouse_pos, 5.0f, glm::vec4(1,1,1,1));
  }
  if (glfwGetKey(context, GLFW_KEY_5) == GLFW_PRESS) {
    particle_manager.add(mouse_pos, circle_size, glm::vec4(1,1,1,1));
  }
}
