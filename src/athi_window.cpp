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
#include "athi_settings.h"

#include "Utility/console.h" // console

#include "athi_utility.h" // FRED

#include "./Renderer/opengl_utility.h" // MessageCallback
#include "./Renderer/athi_renderer.h" // render_call

#include "Utility/fixed_size_types.h" // u32, s32, etc.

static std::string title{"Athi"};
static GLFWwindow* window;

void glfw_error_callback(int error, const char* description)
{
    console->error("{}", description);
}


void set_window_resolution(int width, int height)
{

}

GLFWwindow* get_glfw_window()
{
  return window;
}

std::vector<GLFWvidmode> get_video_modes(GLFWmonitor* monitor) noexcept
{
  if (!monitor)
  {
    monitor = glfwGetPrimaryMonitor();
  }

  int count = 0;
  auto temp_video_modes = glfwGetVideoModes(monitor, &count);

  std::vector<GLFWvidmode> video_modes;
  for (int i = 0; i < count; ++i)
  {
    video_modes.emplace_back(temp_video_modes[i]);
  }

  return video_modes;
}

static bool is_same_GLFWvidmode(GLFWvidmode a, GLFWvidmode b) noexcept
{
  return (
    (a.width == b.width) &&
    (a.height == b.height) &&
    (a.refreshRate == b.refreshRate) &&
    (a.redBits == b.redBits) &&
    (a.greenBits == b.greenBits) &&
    (a.greenBits == b.greenBits) &&
    (a.blueBits == b.blueBits)
  );
}

void print_video_modes() noexcept
{
  const auto video_modes = get_video_modes();
  const auto current_video_mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  console->info("Video modes:");
  for (const auto &vm: video_modes)
  {
    if (is_same_GLFWvidmode(vm, *current_video_mode)) console->info("\x1B[32m  {}x{}  {}hz R{}G{}B{}\x1B[0m", vm.width, vm.height, vm.refreshRate, vm.redBits, vm.greenBits, vm.blueBits);
    else console->info("  {}x{}  {}hz R{}G{}B{}", vm.width, vm.height, vm.refreshRate, vm.redBits, vm.greenBits, vm.blueBits);
  }
}

void init_window()
{

  if (!glfwInit())
  {
    console->error("Error initializing GLFW!");
  }

  //glfwWindowHint(GLFW_SAMPLES, 4);
  //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // not functional yet, add a background color pass
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
  glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
  glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

  // Gather monitor info
  print_video_modes();
  auto video_modes = get_video_modes();

  const auto &vm = video_modes[video_modes.size()-1];
  glfwWindowHint(GLFW_RED_BITS, vm.redBits);
  glfwWindowHint(GLFW_GREEN_BITS, vm.greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, vm.blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, vm.refreshRate);

  glfwSetErrorCallback(glfw_error_callback);

  monitor_refreshrate = vm.refreshRate;

  auto primary_monitor = glfwGetPrimaryMonitor();
  auto monitor_name = glfwGetMonitorName(primary_monitor);

  console->info("{} {}x{}({}hz)", monitor_name, vm.width, vm.height, monitor_refreshrate);

  window = glfwCreateWindow(screen_width, screen_height, title.c_str(), NULL, NULL);
  glfwMakeContextCurrent(window);
  glfwSetWindowPos(window, window_pos.x, window_pos.y);

  // glfwSetWindowAspectRatio(window, 1, 1);
  glfwSetWindowSizeCallback(window, window_size_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetWindowPosCallback(window, window_pos_callback);

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
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  framebuffer_width = width;
  framebuffer_height = height;
  camera.update_projection(width, height);
  camera.update();

  {
    float xx;
    glfwGetWindowContentScale(window, &xx, NULL);
    px_scale = xx;
  }


  // During init, enable debug output
  //glEnable              ( GL_DEBUG_OUTPUT );
  //glDebugMessageCallback( (GLDEBUGPROC) MessageCallback, 0 ); OpenGL 4.3 needed

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  check_gl_error();
}
GLFWwindow *get_window_context() { return window; }

void window_pos_callback(GLFWwindow* window, int xpos, int ypos)
{
  window_pos.x = xpos;
  window_pos.y = ypos;
  // console->info("window pos: {}x{}", xpos, ypos);
}

void window_size_callback(GLFWwindow *window, s32 xpos, s32 ypos) {
  // console->info("window size: {}x{}", xpos, ypos);
  screen_width = xpos;
  screen_height = ypos;
}

void framebuffer_size_callback(GLFWwindow *window, s32 width, s32 height) {
  framebuffer_width = width;
  framebuffer_height = height;
  camera.update_projection(static_cast<float>(width), static_cast<float>(height));
  camera.update();

  render_call([width, height]()
    {
      glViewport(0.0f, 0.0f, width, height);

      // @Hack: this doesnt look right
      for (auto& framebuffer: framebuffers) {
        framebuffer.resize(width, height);
    }
  });

  // @Hack
  if (uniformgrid_parts != 4)
    uniformgrid_parts = 4;
  else
    uniformgrid_parts = 16;

  {
    s32 w, h;
    glfwGetWindowSize(window, &w, &h);
    float xx;
    glfwGetWindowContentScale(window, &xx, NULL);
    px_scale = xx;
    // console->info("framebuffer: {}x{} | pixel scale: {}", width, height, px_scale);
  }
}
