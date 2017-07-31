#include "athi_window.h"
#include "athi_settings.h"

#include <iostream>

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void Athi_Window::init()
{
  if (!glfwInit())
  {
    std::cerr << "Error initializing GLFW!\n";
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  glfwWindowHint(GLFW_SAMPLES, 8);
  glfwWindowHint(GL_RED_BITS, 8);
  glfwWindowHint(GL_GREEN_BITS, 8);
  glfwWindowHint(GL_BLUE_BITS, 8);
  glfwWindowHint(GL_ALPHA_BITS, 8);
  glfwWindowHint(GL_BUFFER_SIZE, 32);
  glfwWindowHint(GL_DEPTH_BITS, 16);
  glfwWindowHint(GL_DOUBLEBUFFER, 1);

  context = glfwCreateWindow(scene.width, scene.height, title.c_str(), NULL, NULL);
  glfwMakeContextCurrent(context);

  glfwSetFramebufferSizeCallback(context, framebuffer_size_callback);

  // Gather monitor info
  s32  count;
  auto modes  = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
  monitor_refreshrate = modes->refreshRate;

  // Setup GLEW
  glewExperimental = true;
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "Error initializing GLEW!\n";
  }
}

void Athi_Window::open()
{

}

void Athi_Window::update()
{
}

GLFWwindow* Athi_Window::get_window_context()
{
  return context;
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  std::cout << "viewport not changed: screen: " << width << "x" << height << std::endl;
}
