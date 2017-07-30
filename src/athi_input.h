#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "athi_typedefs.h"
#include <iostream>
#include "athi_settings.h"


void init_input_manager();
u8 get_mouse_button_state(u8 button);
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
  if (key == GLFW_KEY_I && action == GLFW_PRESS)
  {
    if (show_settings) show_settings = false;
    else show_settings = true;
  }
}
