
#include "../dep/Universal/imgui.h"
#include "athi_camera.h"
#include "athi_settings.h"
#include "athi_window.h"

void Athi_Window::init() {

  if (!glfwInit()) {
    console->error("Error initializing GLFW!");
  }

  glfwWindowHint(GLFW_SAMPLES, 0);
  // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // Gather monitor info
  std::int32_t count;
  auto modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
  monitor_refreshrate = modes->refreshRate;
  framerate_limit = monitor_refreshrate;

  auto monitor_name = glfwGetMonitorName(glfwGetPrimaryMonitor());

  console->info("Monitor: {} {}hz", monitor_name, monitor_refreshrate);

  context = glfwCreateWindow(scene.width, scene.height, title.c_str(), NULL, NULL);
  glfwMakeContextCurrent(context);

  // glfwSetWindowAspectRatio(context, 1, 1);
  glfwSetWindowSizeCallback(context, window_size_callback);
  glfwSetFramebufferSizeCallback(context, framebuffer_size_callback);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glDisable(GL_DEPTH_BUFFER);

  std::int32_t width, height;
  glfwGetFramebufferSize(context, &width, &height);
  glViewport(0, 0, width, height);
  screen_width = width;
  screen_height = height;
  camera.update_projection(width, height);
  camera.update();

  // GLEW setup / experimental because of glew bugs
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    console->error("Error initializing GLEW!");
  }
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

  framebuffer->resize(width, height);

  // @Hack
  if (voxelgrid_parts != 4)
    voxelgrid_parts = 4;
  else
    voxelgrid_parts = 16;

  std::int32_t w, h;
  glfwGetWindowSize(window, &w, &h);
  px_scale = static_cast<float>(width) / static_cast<float>(w);

  console->info("framebuffer: {}x{} | pixel scale: {}", width, height, px_scale);

  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = 1.0f / px_scale;
  //
}
