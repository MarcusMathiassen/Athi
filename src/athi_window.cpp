
#include "../dep/Universal/imgui.h"
#include "athi_camera.h"
#include "athi_settings.h"
#include "athi_window.h"
#include "athi_utility.h"

void Athi_Window::init() {

  if (!glfwInit()) {
    console->error("Error initializing GLFW!");
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // not functional yet, add a background color pass
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // Gather monitor info
  std::int32_t count;
  auto modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
  glfwWindowHint(GLFW_RED_BITS, modes->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, modes->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, modes->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, modes->refreshRate);

  monitor_refreshrate = modes->refreshRate;

  auto monitor_name = glfwGetMonitorName(glfwGetPrimaryMonitor());

  console->info("{} {} {}hz", FRED("Monitor:"), monitor_name, monitor_refreshrate);

  context = glfwCreateWindow(scene.width, scene.height, title.c_str(), NULL, NULL);
  glfwMakeContextCurrent(context);

  // glfwSetWindowAspectRatio(context, 1, 1);
  glfwSetWindowSizeCallback(context, window_size_callback);
  glfwSetFramebufferSizeCallback(context, framebuffer_size_callback);

  // GLEW setup / experimental because of glew bugs
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    console->error("Error initializing GLEW!");
  }

  std::int32_t width, height;
  glfwGetFramebufferSize(context, &width, &height);
  glViewport(0, 0, width, height);
  screen_width = width;
  screen_height = height;
  camera.update_projection(width, height);
  camera.update();

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glDisable(GL_DEPTH_BUFFER);

  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

GLFWwindow *Athi_Window::get_window_context() { return context; }

void Athi_Window::window_size_callback(GLFWwindow *window, std::int32_t xpos, std::int32_t ypos) {
  console->info("window size: {}x{}", xpos, ypos);
}

void Athi_Window::framebuffer_size_callback(GLFWwindow *window, std::int32_t width, std::int32_t height) {
  screen_width = width;
  screen_height = height;
  camera.update_projection(static_cast<float>(width), static_cast<float>(height));
  camera.update();
  glViewport(0.0f, 0.0f, width, height);


  // @Hack: this doesnt look right
  for (auto& framebuffer: framebuffers) {
    framebuffer.resize(width, height);
  }

  // @Hack
  if (uniformgrid_parts != 4)
    uniformgrid_parts = 4;
  else
    uniformgrid_parts = 16;

  std::int32_t w, h;
  glfwGetWindowSize(window, &w, &h);
  px_scale = static_cast<float>(width) / static_cast<float>(w);

  console->info("framebuffer: {}x{} | pixel scale: {}", width, height, px_scale);
}
