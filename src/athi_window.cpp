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

static vector<GLFWwindow*> windows;

void glfw_error_callback(int error, const char* description)
{
    console->error("{}", description);
}

void profiler_window_close_callback(GLFWwindow* window)
{
  glfwSetWindowShouldClose(window, GLFW_TRUE);
  glfwHideWindow(window);
}

void primary_window_close_callback(GLFWwindow* window)
{
  glfwSetWindowShouldClose(window, GLFW_TRUE);
  glfwSetWindowShouldClose(windows[1], GLFW_TRUE);
}

void open_profiler_window() noexcept
{
  int left, right;

  // if already created
  if (windows[1] != nullptr)
  {
    if (glfwGetWindowAttrib(windows[1], GLFW_VISIBLE))
      glfwHideWindow(windows[1]);
    else {
      glfwShowWindow(windows[1]);
      glfwFocusWindow(windows[0]);

      // Set the profilers position next to the primary window
      glfwSetWindowSize(windows[1], screen_width, screen_height);
      glfwSetWindowPos(windows[1], window_pos.x + screen_width, window_pos.y);
    }
    return;
  }
}

void update_windows()
{
  for (auto & window: windows) {
    if (window) {
      if (glfwWindowShouldClose(window)) {
        glfwDestroyWindow(window);
        window = nullptr;
      }
    }
  }
}

void init_window() {

  if (!glfwInit()) {
    console->error("Error initializing GLFW!");
  }

  //glfwWindowHint(GLFW_SAMPLES, 4);
  //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // not functional yet, add a background color pass
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
  glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

  glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);


  // Gather monitor info
  s32 count;
  auto modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);
  console->info("RGBA bits: {}{}{}{}", modes->redBits, modes->greenBits, modes->blueBits, 8);
  glfwWindowHint(GLFW_RED_BITS, modes->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, modes->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, modes->blueBits);
  glfwWindowHint(GLFW_ALPHA_BITS, 8);
  glfwWindowHint(GLFW_REFRESH_RATE, modes->refreshRate);

  glfwSetErrorCallback(glfw_error_callback);

  monitor_refreshrate = modes->refreshRate;

  auto monitor_name = glfwGetMonitorName(glfwGetPrimaryMonitor());

  console->info("{} {} {}hz", FRED("Monitor:"), monitor_name, monitor_refreshrate);

  windows.resize(2);

  windows[0] = glfwCreateWindow(screen_width, screen_height, title.c_str(), NULL, NULL);
  glfwMakeContextCurrent(windows[0]);
  glfwSetWindowPos(windows[0], window_pos.x, window_pos.y);

  glfwWindowHint(GLFW_FOCUSED, false);
  glfwWindowHint(GLFW_VISIBLE, false);
  windows[1] = glfwCreateWindow(screen_width, screen_height, "Profiler", NULL, windows[0]);
  glfwSetWindowCloseCallback(windows[1], profiler_window_close_callback);

  // Set the profilers position next to the primary window
  glfwSetWindowSize(windows[1], screen_width, screen_height);
  glfwSetWindowPos(windows[1], window_pos.x + screen_width + screen_width, window_pos.y);


  // glfwSetWindowAspectRatio(windows[0], 1, 1);
  glfwSetWindowSizeCallback(windows[0], window_size_callback);
  glfwSetFramebufferSizeCallback(windows[0], framebuffer_size_callback);
  glfwSetWindowPosCallback(windows[0], window_pos_callback);
  glfwSetWindowCloseCallback(windows[0], primary_window_close_callback);

  // GLEW setup / experimental because of glew bugs
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    console->error("Error initializing GLEW!");
  }

  int param[10];
  glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &param[0]);
  glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &param[1]);
  glGetIntegerv(GL_PRIMITIVE_RESTART, &param[2]);
  glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &param[3]);
  glGetIntegerv(GL_MAX_SAMPLES, &param[4]);
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &param[5]);
  console->info("GL_MAX_ELEMENTS_INDICES:          {}", param[0]);
  console->info("GL_MAX_ELEMENTS_VERTICES:         {}", param[1]);
  console->info("GL_PRIMITIVE_RESTART:             {}", param[2]);
  console->info("GL_MAX_SAMPLES:                   {}", param[4]);
  console->info("GL_MAX_COLOR_ATTACHMENTS:         {}", param[5]);
  console->info("GL_NUM_SHADING_LANGUAGE_VERSIONS: {}", glGetString(GL_SHADING_LANGUAGE_VERSION));


  s32 width, height;
  glfwGetFramebufferSize(windows[0], &width, &height);
  glViewport(0, 0, width, height);
  framebuffer_width = width;
  framebuffer_height = height;
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

GLFWwindow *get_window_context(int i) { return windows[i]; }

void window_pos_callback(GLFWwindow* window, int xpos, int ypos)
{
  window_pos.x = xpos;
  window_pos.y = ypos;
  //console->info("window pos: {}x{}", xpos, ypos);
}

void window_size_callback(GLFWwindow *window, s32 xpos, s32 ypos) {
  //console->info("window size: {}x{}", xpos, ypos);
  screen_width = xpos;
  screen_height = ypos;
}

void framebuffer_size_callback(GLFWwindow *window, s32 width, s32 height) {
  framebuffer_width = width;
  framebuffer_height = height;
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

  //console->info("framebuffer: {}x{} | pixel scale: {}", width, height, px_scale);

  check_gl_error();
}
