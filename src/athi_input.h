#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "athi_particle.h"
#include "athi_camera.h"
#include "athi_circle.h"
#include "athi_rect.h"
#include "athi_settings.h"
#include "athi_typedefs.h"

void init_input_manager();
vec2 get_mouse_viewspace_pos();
u8 get_mouse_button_state(u8 button);
void update_inputs();
static void mouse_button_callback(GLFWwindow *window, int button, int action,
                                  int mods);
static void cursor_position_callback(GLFWwindow *window, double xpos,
                                     double ypos);
static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods);
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

struct Mouse {
  vec2 pos;
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
    glfwSetCursorPosCallback(context, cursor_position_callback);
    glfwSetScrollCallback(context, scroll_callback);
  }
};

extern Athi_Input_Manager athi_input_manager;

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  mouse_size -= yoffset * 0.001f;
  if (mouse_size < 0.000f) mouse_size = 0.001f;
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
  athi_input_manager.mouse.pos.x = xpos;
  athi_input_manager.mouse.pos.y = ypos;
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      athi_input_manager.mouse.left_button.state = action;
    case GLFW_MOUSE_BUTTON_RIGHT:
      athi_input_manager.mouse.right_button.state = action;
  }

  s32 width, height;
  glm::vec2 mouse_pos = athi_input_manager.mouse.pos;
  glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
  mouse_pos.x = -1.0f + 2 * mouse_pos.x / width;
  mouse_pos.y = +1.0f - 2 * mouse_pos.y / height;
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    particle_manager.add(mouse_pos, circle_size, glm::vec4(1,1,1,1));
  }
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  // TOGGLE SETTINGS UI
  if (key == GLFW_KEY_I && action == GLFW_PRESS) {
    show_settings ^= 1;
  }

  // TOGGLE SLOWMOTION
  if (key == GLFW_KEY_O && action == GLFW_PRESS) {
    slowmotion ^= 1;
  }

  // TOGGLE FPS_INFO
  if (key == GLFW_KEY_F && action == GLFW_PRESS) {
    show_fps_info ^= 1;
  }

  // TOGGLE VSYNC
  if (key == GLFW_KEY_L && action == GLFW_PRESS) {
    vsync ^= 1;
  }

  // TOGGLE DEBUG UI
  if (key == GLFW_KEY_D && action == GLFW_PRESS) {
    draw_debug ^= 1;
  }

  // TOGGLE CIRCLE GRAVITY
  if (key == GLFW_KEY_G && action == GLFW_PRESS) {
    physics_gravity ^= 1;
  }

  // TOGGLE CIRCLE COLLISIONS
  if (key == GLFW_KEY_C && action == GLFW_PRESS) {
    circle_collision ^= 1;
  }

  // TOGGLE MULTITHREADING
  if (key == GLFW_KEY_M && action == GLFW_PRESS) {
    use_multithreading ^= 1;
  }

  // TOGGLE OPENCL
  if (key == GLFW_KEY_A && action == GLFW_PRESS) {
    openCL_active ^= 1;
  }

  // TOGGLE MOUSE GRAB LINES
  if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
    show_mouse_grab_lines ^= 1;
  }

  // TOGGLE QUADTREE ACTIVE
  if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
    if (quadtree_active)
      quadtree_active = false;
    else {
      quadtree_active = true;
      voxelgrid_active = false;
    }
  }

  // TOGGLE VOXELGRID ACTIVE
  if (key == GLFW_KEY_W && action == GLFW_PRESS) {
    if (voxelgrid_active)
      voxelgrid_active = false;
    else {
      voxelgrid_active = true;
      quadtree_active = false;
    }
  }

  if (key == GLFW_KEY_6 && action == GLFW_PRESS && voxelgrid_parts > 4) {
    voxelgrid_parts *= 0.25f;
  }

  if (key == GLFW_KEY_7 && action == GLFW_PRESS && voxelgrid_parts < 512) {
    voxelgrid_parts *= 4;
  }

  // TEST
  if (key == GLFW_KEY_B && action == GLFW_PRESS) {
    Athi_Circle c;
    c.radius = circle_size;
    for (float i = -1 + c.radius; i < 1; i += c.radius * 2)
      for (float j = -1 + c.radius; j < 1; j += c.radius * 2) {
        c.pos = vec2(i, j);
        add_circle(c);
      }
  }

  // ERASE ALL CIRCLES
  if (key == GLFW_KEY_E && action == GLFW_PRESS) {
    delete_circles();
  }
}
