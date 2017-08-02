#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "athi_typedefs.h"
#include <iostream>
#include "athi_settings.h"
#include "athi_circle.h"

void init_input_manager();
u8 get_mouse_button_state(u8 button);
static void update_inputs();
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

struct Mouse
{
  vec2 pos;
  struct Button{bool state;} left_button, right_button;
};

struct Athi_Input_Manager
{
  Mouse mouse;
  void init()
  {
    glfwSetMouseButtonCallback(glfwGetCurrentContext(), mouse_button_callback);
    glfwSetKeyCallback(glfwGetCurrentContext(), key_callback);
    glfwSetCursorPosCallback(glfwGetCurrentContext(), cursor_position_callback);
  }
};

static Athi_Input_Manager athi_input_manager;

static void update_inputs()
{
  f64 mouse_x, mouse_y;
  glfwGetCursorPos(glfwGetCurrentContext(), &mouse_x, &mouse_y);

  s32 width, height;
  glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
  mouse_x = -1.0f + 2 * mouse_x / width;
  mouse_y = +1.0f - 2 * mouse_y / height;

  Athi_Circle c;
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_SPACE) == GLFW_PRESS)
  {
    c.pos = vec2(mouse_x,mouse_y);
    c.radius = circle_size;
    addCircle(c);
  }
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_1) == GLFW_PRESS)
  {
    c.pos = vec2(mouse_x,mouse_y);
    c.radius = 0.01f;
    addCircle(c);
  }
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_2) == GLFW_PRESS)
  {
    c.pos = vec2(mouse_x,mouse_y);
    c.radius = 0.02f;
    addCircle(c);
  }
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_3) == GLFW_PRESS)
  {
    c.pos = vec2(mouse_x,mouse_y);
    c.radius = 0.03f;
    addCircle(c);
  }

}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
  athi_input_manager.mouse.pos.x = xpos;
  athi_input_manager.mouse.pos.y = ypos;
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  switch(button)
  {
    case GLFW_MOUSE_BUTTON_LEFT: athi_input_manager.mouse.left_button.state = action;
    case GLFW_MOUSE_BUTTON_RIGHT: athi_input_manager.mouse.right_button.state = action;
  }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  // TOGGLE SETTINGS UI
  if (key == GLFW_KEY_I && action == GLFW_PRESS)
  {
    if (show_settings) show_settings = false;
    else show_settings = true;
  }

  // TOGGLE CIRCLE GRAVITY
  if (key == GLFW_KEY_G && action == GLFW_PRESS)
  {
    if (physics_gravity) physics_gravity = false;
    else physics_gravity = true;
  }

  // TOGGLE CIRCLE COLLISIONS
  if (key == GLFW_KEY_C && action == GLFW_PRESS)
  {
    if (circle_collision) circle_collision = false;
    else circle_collision = true;
  }

  // TOGGLE QUADTREE ACTIVE
  if (key == GLFW_KEY_Q && action == GLFW_PRESS)
  {
    if (quadtree_active) quadtree_active = false;
    else { reset_quadtree(); quadtree_active = true; voxelgrid_active = false; }
  }

  // TOGGLE VOXELGRID ACTIVE
  if (key == GLFW_KEY_W && action == GLFW_PRESS)
  {
    if (voxelgrid_active) voxelgrid_active = false;
    else {voxelgrid_active = true; quadtree_active = false; }
  }

  // TOGGLE MULTITHREADING
  if (key == GLFW_KEY_M && action == GLFW_PRESS)
  {
    if (use_multithreading) use_multithreading = false;
    else use_multithreading = true;
  }

  if (key == GLFW_KEY_6 && action == GLFW_PRESS && voxelgrid_parts > 4)
  {
    voxelgrid_parts *= 0.25f;
  }

  if (key == GLFW_KEY_7 && action == GLFW_PRESS && voxelgrid_parts < 64)
  {
    voxelgrid_parts *= 4;
  }

  // clear all circles
  if (key == GLFW_KEY_E && action == GLFW_PRESS)
  {
    delete_circles();
  }
}
