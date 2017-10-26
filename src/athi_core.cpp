#define STB_IMAGE_IMPLEMENTATION

#include "athi_core.h"
#include "athi_camera.h"
#include "athi_input.h"
#include "athi_line.h"
#include "athi_rect.h"
#include "athi_renderer.h"
#include "athi_settings.h"
#include "athi_spring.h"
#include "athi_utility.h"
#include "athi_gui.h"

#include <array>
#include <dispatch/dispatch.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "spdlog/spdlog.h"

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

Smooth_Average<double, 30> smooth_frametime_avg(&smoothed_frametime);
Smooth_Average<double, 30>
    smooth_physics_rametime_avg(&smoothed_physics_frametime);
Smooth_Average<double, 30>
    smooth_render_rametime_avg(&smoothed_render_frametime);

void Athi_Core::init() {

  window.scene.width = 1280;
  window.scene.height = 800;
  window.init();

  particle_manager.init();

  init_input_manager();
  init_rect_manager();
  init_line_manager();

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_BUFFER);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.15686, 0.17255, 0.20392, 1.0);

  variable_thread_count = std::thread::hardware_concurrency();

  int width, height;
  glfwGetFramebufferSize(window.get_window_context(), &width, &height);
  px_scale = static_cast<float>(width) / static_cast<float>(window.scene.width);

  gui_init(window.get_window_context(), px_scale);

  auto console = spdlog::stdout_color_mt("Athi");
  console->info("Initializing Athi..");
  console->info("CPU: {}", get_cpu_brand());
  console->info("Threads available: {}", get_cpu_threads());
  console->info("IMGUI VERSION {}", ImGui::GetVersion());
  console->info("GL_VERSION {}", glGetString(GL_VERSION));
  console->info("GL_VENDOR {}", glGetString(GL_VENDOR));
  console->info("GL_RENDERER {}", glGetString(GL_RENDERER));
  console->info("Using GLEW {}", glewGetString(GLEW_VERSION));
  console->info("Using GLFW {}", glfwGetVersionString());
}

void Athi_Core::start() {
  auto window_context = window.get_window_context();
  glfwMakeContextCurrent(window_context);
  while (!glfwWindowShouldClose(window_context)) {
    const double time_start_frame = glfwGetTime();

    glfwPollEvents();
    update_inputs();
    window.update();
    update_settings();

    update();
    draw(window_context);

    if (framerate_limit != 0) limit_FPS(framerate_limit, time_start_frame);
    frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    framerate = static_cast<uint32_t>(std::round(1000.0f / smoothed_frametime));
    smooth_frametime_avg.add_new_frametime(frametime);
  }

  app_is_running = false;
  shutdown();
}


void Athi_Core::draw(GLFWwindow *window) {
  const double time_start_frame = glfwGetTime();
  glClearColor(background_color.x, background_color.y, background_color.z,
               background_color.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  particle_manager.draw();
  draw_rects();
  draw_lines();
  render();

  if (show_settings) {
    gui_render();
  }

  render_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  render_framerate =
      static_cast<uint32_t>(std::round(1000.0f / smoothed_render_frametime));
  smooth_render_rametime_avg.add_new_frametime(render_frametime);

  glfwSwapBuffers(window);
}

void Athi_Core::update() {
  const double time_start_frame = glfwGetTime();
  int iter = 0;
  while (iter++ < physics_samples) {
    const double start = glfwGetTime();
    particle_manager.update();
    timestep = (((glfwGetTime() - start) * 1000.0) / (1000.0 / 60.0)) / physics_samples;
  }
  physics_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  physics_framerate =
      static_cast<uint32_t>(std::round(1000.0f / smoothed_physics_frametime));
  smooth_physics_rametime_avg.add_new_frametime(physics_frametime);
}

void Athi_Core::update_settings() { glfwSwapInterval(vsync); }

void Athi_Core::shutdown() {
  gui_shutdown();
  glfwTerminate();
}
