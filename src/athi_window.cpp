
#include <iostream>

#include "../dep/Universal/imgui.h"
#include "athi_camera.h"
#include "athi_settings.h"
#include "athi_window.h"

void Athi_Window::init()
{

  if (!glfwInit())
  {
    std::cerr << "Error initializing GLFW!\n";
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  context = glfwCreateWindow(scene.width, scene.height, title.c_str(), NULL, NULL);
  glfwMakeContextCurrent(context);

  // glfwSetWindowAspectRatio(context, 1, 1);
  glfwSetWindowSizeCallback(context, window_size_callback);
  glfwSetFramebufferSizeCallback(context, framebuffer_size_callback);

  std::int32_t width, height;
  glfwGetFramebufferSize(context, &width, &height);
  glViewport(0, 0, width, height);
  screen_width = width;
  screen_height = height;
  camera.update_projection(width, height);
  camera.update();

  // Gather monitor info
  std::int32_t count;
  auto modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
  monitor_refreshrate = modes->refreshRate;

  // Setup GLEW
  glewExperimental = true;
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "Error initializing GLEW!\n";
  }
}

void Athi_Window::open() {}

void Athi_Window::update() {}

GLFWwindow *Athi_Window::get_window_context() { return context; }

void Athi_Window::window_size_callback(GLFWwindow *window, std::int32_t xpos, std::int32_t ypos)
{
  std::cout << "window size: " << xpos << "x" << ypos << '\n';
}

void Athi_Window::framebuffer_size_callback(GLFWwindow *window, std::int32_t width, std::int32_t height)
{
  screen_width = width;
  screen_height = height;
  camera.update_projection(static_cast<float>(width), static_cast<float>(height));
  camera.update();
  glViewport(0.0f, 0.0f, width, height);

  std::cout << "framebuffer: " << width << "x" << height << '\n';

  // @Hack
  if (voxelgrid_parts != 4)
    voxelgrid_parts = 4;
  else
    voxelgrid_parts = 16;

  std::int32_t w, h;
  glfwGetWindowSize(window, &w, &h);
  px_scale = static_cast<float>(width) / static_cast<float>(w);
  std::cout << "pxscale: " << px_scale << '\n';

  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = 1.0f / px_scale;
  //
}
