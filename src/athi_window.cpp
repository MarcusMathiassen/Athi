// Copyright (c) 2018 Marcus Mathiassen

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.


#include "athi_window.h" 

#include "../dep/Universal/imgui.h" // ImGUI
#include "./Renderer/athi_camera.h" // camera
#include "athi_settings.h" // console
#include "athi_utility.h" // FRED
#include "./Renderer/opengl_utility.h" // MessageCallback

void glfw_error_callback(int error, const char* description)
{
    console->error("{}", description);
}

void Athi_Window::init() {

  if (!glfwInit()) {
    console->error("Error initializing GLFW!");
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // not functional yet, add a background color pass
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // Gather monitor info
  s32 count;
  auto modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
  glfwWindowHint(GLFW_RED_BITS, modes->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, modes->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, modes->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, modes->refreshRate);

  glfwSetErrorCallback(glfw_error_callback);

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

  s32 width, height;
  glfwGetFramebufferSize(context, &width, &height);
  glViewport(0, 0, width, height);
  screen_width = width;
  screen_height = height;
  camera.update_projection(width, height);
  camera.update();

  // During init, enable debug output
  //glEnable              ( GL_DEBUG_OUTPUT );
  //glDebugMessageCallback( (GLDEBUGPROC) MessageCallback, 0 ); OpenGL 4.3 needed

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  check_gl_error();
}

GLFWwindow *Athi_Window::get_window_context() { return context; }

void Athi_Window::window_size_callback(GLFWwindow *window, s32 xpos, s32 ypos) {
  console->info("window size: {}x{}", xpos, ypos);
}

void Athi_Window::framebuffer_size_callback(GLFWwindow *window, s32 width, s32 height) {
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

  s32 w, h;
  glfwGetWindowSize(window, &w, &h);

  px_scale = static_cast<float>(width) / static_cast<float>(w);

  console->info("framebuffer: {}x{} | pixel scale: {}", width, height, px_scale);

  check_gl_error();
}
