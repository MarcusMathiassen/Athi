#include "athi_core.h"

#define STB_IMAGE_IMPLEMENTATION

#include "athi_typedefs.h"
#include "athi_gui.h" // gui_init, gui_render, gui_shutdown
#include "athi_input.h" // update_inputs
#include "athi_line.h" // draw_lines
#include "athi_rect.h" // draw_rects
#include "athi_renderer.h" // render
#include "athi_settings.h" // console, ThreadPoolSolution
#include "athi_utility.h" // profile, Smooth_Average

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../dep/Universal/imgui.h"
#include "../dep/Universal/imgui_impl_glfw_gl3.h"
#include "../dep/Universal/spdlog/spdlog.h"

static Smooth_Average<f64, 30> smooth_frametime_avg(&smoothed_frametime);
static Smooth_Average<f64, 30> smooth_physics_rametime_avg(&smoothed_physics_frametime);
static Smooth_Average<f64, 30> smooth_render_rametime_avg(&smoothed_render_frametime);

void Athi_Core::init() {
  spdlog::set_pattern("[%H:%M:%S] %v");
  console = spdlog::stdout_color_mt("Athi");
  if constexpr (ONLY_RUNS_IN_DEBUG_MODE) console->critical("DEBUG MODE: ON");

  window.scene.width = 512;
  window.scene.height = 512;
  window.init();

  // Apple specific settings
#if __APPLE__
  use_libdispatch = true;
  threadpool_solution = ThreadPoolSolution::AppleGCD;
#endif

  int width, height;
  glfwGetFramebufferSize(window.get_window_context(), &width, &height);
  px_scale = static_cast<f32>(width) / static_cast<f32>(window.scene.width);

  gui_init(window.get_window_context(), px_scale);
  variable_thread_count = std::thread::hardware_concurrency();

  // Debug information
  console->info("{} {}", FRED("CPU:"), get_cpu_brand());
  console->info("Threads available: {}", std::thread::hardware_concurrency());
  console->info("IMGUI VERSION {}", ImGui::GetVersion());
  //console->info("GLM VERSION {}", glm::get_version());
  console->info("GL_VERSION {}", glGetString(GL_VERSION));
  console->info("GL_VENDOR {}", glGetString(GL_VENDOR));
  console->info("GL_RENDERER {}", glGetString(GL_RENDERER));
  console->info("Using GLEW {}", glewGetString(GLEW_VERSION));
  console->info("Using GLFW {}", glfwGetVersionString());

  particle_system.init();

  init_input_manager();
  init_rect_manager();
  init_line_manager();

  glClearColor(background_color_dark.r, background_color_dark.g,
               background_color_dark.b, background_color_dark.a);
}

void Athi_Core::start() {
  auto window_context = window.get_window_context();
  glfwMakeContextCurrent(window_context);

  setup_fullscreen_quad();

  framebuffers.resize(2);
  framebuffers[0].resize(screen_width, screen_height);
  framebuffers[1].resize(screen_width, screen_height);

  while (!glfwWindowShouldClose(window_context)) {
    const f64 time_start_frame = glfwGetTime();

    glfwPollEvents();
    update_inputs();

    update();

    draw(window_context);

    {
      profile p("glfwSwapBuffers");
      glfwSwapBuffers(window_context);
    }

    if (framerate_limit != 0) limit_FPS(framerate_limit, time_start_frame);
    frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    framerate = static_cast<u32>(std::round(1000.0f / smoothed_frametime));
    smooth_frametime_avg.add_new_frametime(frametime);
  }

  app_is_running = false;
  shutdown();
}

void Athi_Core::draw(GLFWwindow *window) {
  profile p("Athi_Core::draw");

  const f64 time_start_frame = glfwGetTime();
  glClearColor(background_color_dark.r, background_color_dark.g,
               background_color_dark.b, background_color_dark.a);

  glClear(GL_COLOR_BUFFER_BIT);

  if (post_processing) {
    profile p("post processing");

    framebuffers[0].clear();
    draw_fullscreen_quad(framebuffers[0].texture, vec2(0, 0));

    // First draw the particles to the framebuffer.
    framebuffers[0].bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    particle_system.draw();

    // .. Then blur the current framebuffer
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    for (s32 i = 0; i < post_processing_samples; i++) {
      draw_fullscreen_quad(framebuffers[0].texture,
                           vec2(0, 1 * blur_strength));  // first pass
      draw_fullscreen_quad(framebuffers[0].texture,
                           vec2(1 * blur_strength, 0));  // first pass
    }
  }

  if (post_processing) {
    framebuffers[0].unbind();
    draw_fullscreen_quad(framebuffers[0].texture, vec2(0, 0));
  }

  particle_system.draw();

  draw_rects();
  draw_lines();
  render();

  //@Bug: rects and lines are being drawn over the Gui.
  if (show_settings) {
    update_settings();
    gui_render();
  }

  render_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  render_framerate =
      static_cast<u32>(std::round(1000.0f / smoothed_render_frametime));
  smooth_render_rametime_avg.add_new_frametime(render_frametime);
}

void Athi_Core::update() {
  const f64 time_start_frame = glfwGetTime();

  if (!particle_system.particles.empty()) {
    particle_system.update();
  }

  // Draw nodes and/or color objects
  particle_system.draw_debug_nodes();

  // Update GPU buffers
  particle_system.update_gpu_buffers();

  // Update timers
  physics_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  physics_framerate =
      static_cast<u32>(std::round(1000.0f / smoothed_physics_frametime));
  smooth_physics_rametime_avg.add_new_frametime(physics_frametime);
  timestep = 1.0 / 60.0;
}

void Athi_Core::update_settings() { glfwSwapInterval(vsync); }

void Athi_Core::shutdown() {
  gui_shutdown();
  glfwTerminate();
}
