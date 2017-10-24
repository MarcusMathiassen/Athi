#define STB_IMAGE_IMPLEMENTATION

#include "athi_core.h"
#include "athi_camera.h"
#include "athi_input.h"
#include "athi_line.h"
#include "athi_rect.h"
#include "athi_renderer.h"
#include "athi_settings.h"
#include "athi_spring.h"
#include "athi_text.h"
#include "athi_utility.h"

#include <dispatch/dispatch.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "spdlog/spdlog.h"

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

Smooth_Average<double, 50> smooth_frametime_avg(&smoothed_frametime);
Smooth_Average<double, 50> smooth_physics_rametime_avg(&smoothed_physics_frametime);

namespace spd = spdlog;

void Athi_Core::init() {

  window = std::make_unique<Athi_Window>();
  window->scene.width = 500;
  window->scene.height = 500;
  window->init();

  particle_manager.init();

  init_rect_manager();
  init_line_manager();

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_BUFFER);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.15686, 0.17255, 0.20392, 1.0);

  variable_thread_count = std::thread::hardware_concurrency();  

  auto console = spd::stdout_color_mt("Athi");
  console->info("Initializing Athi..");
  console->info("CPU: {}", get_cpu_brand());
  console->info("Threads available: {}", get_cpu_threads());
  console->info("GL_VERSION {}", glGetString(GL_VERSION));
  console->info("GL_VENDOR {}", glGetString(GL_VENDOR));
  console->info("GL_RENDERER {}", glGetString(GL_RENDERER));
  console->info("Using GLEW {}", glewGetString(GLEW_VERSION));
  console->info("Using GLFW {}", glfwGetVersionString());

  int width, height;
  glfwGetFramebufferSize(window->get_window_context(), &width, &height);
  const float font_retina_scale = static_cast<float>(width) / static_cast<float>(height);
  ImGui_ImplGlfwGL3_Init(window->get_window_context(), true);
  ImGuiIO& io = ImGui::GetIO();
  io.FontGlobalScale = 1.0f / font_retina_scale;
  io.Fonts->AddFontFromFileTTF("../Resources/DroidSans.ttf", 12, NULL, io.Fonts->GetGlyphRangesJapanese());
}

void Athi_Core::start() {

#ifdef ATHI_MULTITHREADED
  dispatch_queue_t draw_q = dispatch_queue_create("draw_q", NULL);
  dispatch_queue_t update_q = dispatch_queue_create("update_q", NULL);

  dispatch_async(draw_q, ^{
    draw_loop();
  });

  dispatch_async(update_q, ^{
    physics_loop();
  });

  auto window_context = window->get_window_context();
  auto monitor_refreshrate = window->monitor_refreshrate;
  while (!glfwWindowShouldClose(window_context)) {
    const double time_start_frame = glfwGetTime();
    update_inputs();
    window->update();
    update_settings();
    glfwPollEvents();
    limit_FPS(monitor_refreshrate, time_start_frame);
  }
  app_is_running = false;
  shutdown();

#else // singlethreaded

  auto window_context = window->get_window_context();
  glfwMakeContextCurrent(window_context);
  while (!glfwWindowShouldClose(window_context)) {
    const double time_start_frame = glfwGetTime();
    ImGui_ImplGlfwGL3_NewFrame();
    
    update_inputs();
    window->update();
    update_settings();
    glfwPollEvents();

    update();

    draw(window_context);
    if (framerate_limit != 0) limit_FPS(framerate_limit, time_start_frame);
  }
  app_is_running = false;
  shutdown();
#endif
}

void Athi_Core::update() {
  int32_t iter = 0;
  double time_passed = 0.0;
  while (time_passed < 1000.0/60.0) {
    const double time_start_frame = glfwGetTime();

    particle_manager.update();

    physics_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    physics_framerate = static_cast<unsigned int>(std::round(1000.0f / smoothed_physics_frametime));
    smooth_physics_rametime_avg.add_new_frametime(physics_frametime);
    timestep = smoothed_physics_frametime / (1000.0 / 60.0);
    time_passed += physics_frametime;
    ++iter;
  }
}

void Athi_Core::draw(GLFWwindow *window) {
  const double time_start_frame = glfwGetTime();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw_rects();
  draw_lines();
  draw_springs();

  particle_manager.draw();

  if (true)
  {
    ImGui::Begin("Menu");
    ImGui::Text("%d fps %.3f ms", (int)ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    ImGui::End();
  }

  render();
  ImGui::Render();  
  glfwSwapBuffers(window);

  frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  framerate = static_cast<unsigned int>(std::round(1000.0f / smoothed_frametime));
  smooth_frametime_avg.add_new_frametime(frametime);
}

void Athi_Core::draw_loop() {

  auto window_context = window->get_window_context();
  glfwMakeContextCurrent(window_context);

  while (app_is_running) {
    const double time_start_frame = glfwGetTime();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_rects();
    draw_lines();
    draw_springs();

    render();
    glfwSwapBuffers(window_context);

    if (framerate_limit != 0) limit_FPS(framerate_limit, time_start_frame);
    frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    framerate = static_cast<unsigned int>(std::round(1000.0f / smoothed_frametime));
    smooth_frametime_avg.add_new_frametime(frametime);
  }
}

void Athi_Core::physics_loop() {
  while (app_is_running) {
    const double time_start_frame = glfwGetTime();

    update_circles();

    if (physics_FPS_limit != 0) limit_FPS(physics_FPS_limit, time_start_frame);
    physics_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    physics_framerate = static_cast<uint32_t>(std::round(1000.0f / smoothed_physics_frametime));
    smooth_physics_rametime_avg.add_new_frametime(physics_frametime);
    timestep = smoothed_physics_frametime / (1000.0 / 60.0);
  }
}

void Athi_Core::update_settings() { glfwSwapInterval(vsync); }

void Athi_Core::shutdown() { glfwTerminate(); }
