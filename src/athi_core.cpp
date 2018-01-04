#define STB_IMAGE_IMPLEMENTATION

#include "athi_core.h"
#include "athi_gui.h"
#include "athi_input.h"
#include "athi_line.h"
#include "athi_rect.h"
#include "athi_renderer.h"
#include "athi_settings.h"
#include "athi_utility.h"

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
  window.scene.width = 640;
  window.scene.height = 480;
  window.init();

  particle_manager.init();

  init_input_manager();
  init_rect_manager();
  init_line_manager();

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_BUFFER);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.15686f, 0.17255f, 0.20392f, 1.0f);

  variable_thread_count = std::thread::hardware_concurrency();

  int width, height;
  glfwGetFramebufferSize(window.get_window_context(), &width, &height);
  px_scale = static_cast<float>(width) / static_cast<float>(window.scene.width);

  gui_init(window.get_window_context(), px_scale);

  auto console = spdlog::stdout_color_mt("Athi");
  console->info("Initializing Athi..");
  console->info("CPU: {}", get_cpu_brand());
  console->info("Threads available: {}", std::thread::hardware_concurrency());
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

    // Input
    {
      profile p("Input handling");
      glfwPollEvents();
      update_inputs();
    }

    // Update
    update();

    // Draw
    draw(window_context);

    {
      profile p("glfwSwapBuffers");
      glfwSwapBuffers(window_context);
    }

    if (framerate_limit != 0)
      limit_FPS(framerate_limit, time_start_frame);
    frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    framerate = static_cast<u32>(std::round(1000.0f / smoothed_frametime));
    smooth_frametime_avg.add_new_frametime(frametime);
  }

  app_is_running = false;
  shutdown();
}

void Athi_Core::draw(GLFWwindow *window) {
  profile p("Athi_Core::draw");

  const double time_start_frame = glfwGetTime();
  glClearColor(background_color.x, background_color.y, background_color.z,
               background_color.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  {
    profile p("ParticleManager::draw");
    particle_manager.draw();
  }
  {
    profile p("draw_rects");
    draw_rects();
  }
  {
    profile p("draw_lines");
    draw_lines();
  }
  {
    profile p("render");
    render();
  }
  //@Bug: rects and lines are being drawn over the Gui.
  if (show_settings) {
    update_settings();
    gui_render();
  }

  render_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  render_framerate = static_cast<u32>(std::round(1000.0f / smoothed_render_frametime));
  smooth_render_rametime_avg.add_new_frametime(render_frametime);
}

void Athi_Core::update() {
  const double time_start_frame = glfwGetTime();

  // Iterate for how every many samples
  for (int i = 0; i < physics_samples; ++i) {
    const double start = glfwGetTime();
    particle_manager.update();
    timestep = (((glfwGetTime() - start) * 1000.0) / (1000.0 / 60.0)) /
               physics_samples;
  }

  // Update GPU buffers
  particle_manager.update_gpu_buffers();

  // Update timers
  physics_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  physics_framerate = static_cast<u32>(std::round(1000.0f / smoothed_physics_frametime));
  smooth_physics_rametime_avg.add_new_frametime(physics_frametime);
}

void Athi_Core::update_settings() { glfwSwapInterval(vsync); }

void Athi_Core::shutdown() {
  gui_shutdown();
  glfwTerminate();
}
