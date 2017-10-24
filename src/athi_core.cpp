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
Smooth_Average<double, 50> smooth_render_rametime_avg(&smoothed_render_frametime);

namespace spd = spdlog;

void Athi_Core::init() {

  window.scene.width = 512;
  window.scene.height = 512;
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

  //int width, height;
  //glfwGetFramebufferSize(window.get_window_context(), &width, &height);
  //const float font_retina_scale = static_cast<float>(width) / static_cast<float>(height);
  ImGui_ImplGlfwGL3_Init(window.get_window_context(), false);
  //ImGuiIO& io = ImGui::GetIO();
  //io.FontGlobalScale = 1.0f / font_retina_scale;
  //io.Fonts->AddFontFromFileTTF("../Resources/DroidSans.ttf", 12 * font_retina_scale , NULL, io.Fonts->GetGlyphRangesJapanese());

  auto console = spd::stdout_color_mt("Athi");
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
    framerate = static_cast<unsigned int>(std::round(1000.0f / smoothed_frametime));
    smooth_frametime_avg.add_new_frametime(frametime);
  }

  app_is_running = false;
  shutdown();
}

void Athi_Core::draw(GLFWwindow *window) {
  const double time_start_frame = glfwGetTime();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw_rects();

  particle_manager.draw();

  render();

  if (show_settings)
  {
    ImGui_ImplGlfwGL3_NewFrame();    
    ImGui::Begin("Settings");
    ImGui::Text("Particles: %lu", particle_manager.particles.size());
    ImGui::Text("Total: %d fps %.3f ms", framerate, smoothed_frametime); ImGui::SameLine(); ImGui::Checkbox("VSync", &vsync);
    ImGui::Text("Rendering: %d fps %.3f ms", render_framerate, smoothed_render_frametime);
    ImGui::Text("Updating:  %d fps %.3f ms", physics_framerate, smoothed_physics_frametime);   
    ImGui::Checkbox("Gravity", &physics_gravity);
    ImGui::Checkbox("Collision", &circle_collision);
    ImGui::Checkbox("OpenCL", &openCL_active);
    ImGui::Checkbox("Multithreaded", &use_multithreading); ImGui::SameLine(); ImGui::InputInt("", &variable_thread_count);
    ImGui::Checkbox("Debug", &draw_debug);
    if (variable_thread_count < 0) variable_thread_count = 0;

    int prev_optimizer_used = optimizer_used;
    if (quadtree_active)optimizer_used = 0;
    else if (voxelgrid_active) optimizer_used = 1;
    else optimizer_used = 2;

    ImGui::Separator();
    ImGui::RadioButton("Quadtree", &optimizer_used, 0);
    ImGui::SliderInt("depth", &quadtree_depth, 0, 10);
    ImGui::SliderInt("capacity", &quadtree_capacity, 0, 100);
    ImGui::Separator();
    ImGui::RadioButton("Voxelgrid", &optimizer_used, 1);
    ImGui::SliderInt("nodes", &voxelgrid_parts, 0, 256);
    ImGui::RadioButton("None", &optimizer_used, 2);
    

    switch(optimizer_used) {
      case 0: quadtree_active = true; 
              voxelgrid_active = false; 
              break;
      case 1: 
              voxelgrid_active = true;
              quadtree_active = false;
              break; 
      case 2: 
              voxelgrid_active = false;
              quadtree_active = false;
              break; 
      default: break;
    }

    ImGui::End();
    ImGui::Render();
  }

  render_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  render_framerate = static_cast<unsigned int>(std::round(1000.0f / smoothed_render_frametime));
  smooth_render_rametime_avg.add_new_frametime(render_frametime);

  glfwSwapBuffers(window);
}

void Athi_Core::update() {
  const double time_start_frame = glfwGetTime();

  particle_manager.update();

  physics_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  physics_framerate = static_cast<unsigned int>(std::round(1000.0f / smoothed_physics_frametime));
  smooth_physics_rametime_avg.add_new_frametime(physics_frametime);
  timestep = smoothed_physics_frametime / (1000.0 / 60.0);
}

void Athi_Core::update_settings() { glfwSwapInterval(vsync); }

void Athi_Core::shutdown() {   
  ImGui_ImplGlfwGL3_Shutdown();  
  glfwTerminate(); 
}
