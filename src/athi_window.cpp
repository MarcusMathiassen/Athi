
#include <iostream>

#include "athi_camera.h"
#include "athi_settings.h"
#include "athi_window.h"

void window_size_callback(GLFWwindow *window, int xpos, int ypos);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void Athi_Window::init() {
  if (!glfwInit()) {
    std::cerr << "Error initializing GLFW!\n";
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  context =
      glfwCreateWindow(scene.width, scene.height, title.c_str(), NULL, NULL);
  glfwMakeContextCurrent(context);

  // glfwSetWindowAspectRatio(context, 1,1);
  glfwSetWindowSizeCallback(context, window_size_callback);
  glfwSetFramebufferSizeCallback(context, framebuffer_size_callback);

  int width, height;
  glfwGetFramebufferSize(context, &width, &height);
  glViewport(0, 0, width, height);
  screen_width = width;
  screen_height = height;
  camera.aspect_ratio = (f32)width / (f32)height;
  camera.update_perspective();

  // Gather monitor info
  s32 count;
  auto modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
  monitor_refreshrate = modes->refreshRate;

  // Setup GLEW
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    std::cerr << "Error initializing GLEW!\n";
  }
}

void Athi_Window::open() {}

void Athi_Window::update() {}

GLFWwindow *Athi_Window::get_window_context() { return context; }

// @Cleanup: this is messy
void window_size_callback(GLFWwindow *window, int xpos, int ypos) {
  std::cout << "Window size: " << xpos << ":" << ypos << std::endl;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  camera.aspect_ratio = (f32)width / (f32)height;
  camera.window_width = width;
  screen_width = width;
  screen_height = height;
  camera.window_height = height;
  camera.update_perspective();
  glViewport(0, 0, width, height);

  std::cout << "Framebuffer size: " << width << ":" << height << std::endl;
}
