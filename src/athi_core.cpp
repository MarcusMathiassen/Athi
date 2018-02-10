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

#include "athi_core.h"

#define STB_IMAGE_IMPLEMENTATION

#include "./Utility/athi_save_state.h"   // write_data, read_data
#include "./Utility/athi_config_parser.h"   // init_variables
#include "./Renderer/athi_renderer.h"   // render
#include "./Renderer/athi_text.h"       // draw_text
#include "./Renderer/opengl_utility.h"  // check_gl_error();
#include "./Utility/athi_constant_globals.h"     // os
#include "athi_gui.h"                   // gui_init, gui_render, gui_shutdown
#include "athi_input.h"                 // update_inputs
#include "./Renderer/athi_primitives.h"       // draw_circle, draw_rects, draw_lines
#include "athi_settings.h"              // console, ThreadPoolSolution
#include "athi_typedefs.h"
#include "athi_utility.h"               // profile, Smooth_Average

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../dep/Universal/imgui.h"
#include "../dep/Universal/imgui_impl_glfw_gl3.h"
#include "../dep/Universal/spdlog/spdlog.h"

static Smooth_Average<f64, 30> smooth_frametime_avg(&smoothed_frametime);
static Smooth_Average<f64, 30> smooth_physics_rametime_avg(
    &smoothed_physics_frametime);
static Smooth_Average<f64, 30> smooth_render_rametime_avg(
    &smoothed_render_frametime);

static Renderer renderer;

static void setup_fullscreen_quad() {

  auto &shader = renderer.make_shader();
  shader.sources = {"athi_fullscreen_quad.vert", "athi_fullscreen_quad.frag"};
  shader.uniforms = {"transform", "res", "tex", "dir"};
  shader.preambles = {"blurs.glsl"};

  constexpr u16 indices[6] = {0, 1, 2, 0, 2, 3};
  auto &indices_buffer = renderer.make_buffer("indices");
  indices_buffer.data = (void*)indices;
  indices_buffer.data_size = sizeof(indices);
  indices_buffer.type = buffer_type::element_array;

  renderer.finish();
}

static void draw_fullscreen_quad(u32 texture, const vec2 &dir) {
  CommandBuffer cmd;
  cmd.type = primitive::triangles;
  cmd.count = 6;
  cmd.has_indices = true;

  renderer.bind();

  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, texture);

  const auto proj = camera.get_ortho_projection();
  mat4 trans = proj * Transform().get_model();

  renderer.shader.set_uniform("transform", trans);
  renderer.shader.set_uniform("res", framebuffer_width, framebuffer_height);
  renderer.shader.set_uniform("tex", 0);
  renderer.shader.set_uniform("dir", dir);

  renderer.draw(cmd);
}

void Athi_Core::init() {

  spdlog::set_pattern("[%H:%M:%S] %v");
  console = spdlog::stdout_color_mt("Athi");
  if constexpr (DEBUG_MODE) console->critical("DEBUG MODE: ON");

  init_variables();

  window.scene.width = screen_width;
  window.scene.height = screen_height;
  window.init();
  
  stbi_set_flip_vertically_on_load(true);

  // Apple specific settings
#if __APPLE__
  use_libdispatch = true;
  threadpool_solution = ThreadPoolSolution::AppleGCD;
#endif

  px_scale = static_cast<f32>(framebuffer_width) / static_cast<f32>(screen_width);

  gui_init(window.get_window_context(), px_scale);
  variable_thread_count = std::thread::hardware_concurrency();

  // Debug information
  console->info("{} {}", FRED("CPU:"), get_cpu_brand());
  console->info("Threads available: {}", std::thread::hardware_concurrency());
  console->info("IMGUI VERSION {}", ImGui::GetVersion());
  console->info("GLM VERSION {}", "0.9.8");
  console->info("GL_VERSION {}", glGetString(GL_VERSION));
  console->info("GL_VENDOR {}", glGetString(GL_VENDOR));
  console->info("GL_RENDERER {}", glGetString(GL_RENDERER));
  console->info("Using GLEW {}", glewGetString(GLEW_VERSION));
  console->info("Using GLFW {}", glfwGetVersionString());

  particle_system.init();

  init_circle_renderer();
  init_line_renderer();
  init_rect_renderer();
  init_input_manager();

  custom_gui_init();

  glClearColor(background_color.r, background_color.g,
               background_color.b, background_color.a);
  check_gl_error();
}

void Athi_Core::start() {
  auto window_context = window.get_window_context();
  glfwMakeContextCurrent(window_context);

  setup_fullscreen_quad();

  framebuffers.resize(2);
  framebuffers[0].resize(framebuffer_width, framebuffer_height);
  framebuffers[1].resize(framebuffer_width, framebuffer_height);
  
  std::future<void> update_fut, draw_fut;
  if constexpr (multithreaded_engine) { 
    glfwMakeContextCurrent(NULL);
    update_fut = dispatcher.enqueue(&Athi_Core::physics_loop, this);
    draw_fut = dispatcher.enqueue(&Athi_Core::draw_loop, this);
  }

  while (!glfwWindowShouldClose(window_context))
  {
    const f64 time_start_frame = glfwGetTime();

    glfwPollEvents();

    if constexpr (!multithreaded_engine) { 

      if constexpr (DEBUG_MODE) reload_variables();
      
      update_inputs();
      update();
      draw(window_context);
    }

    if (framerate_limit != 0) limit_FPS(framerate_limit, time_start_frame);
    frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    framerate = static_cast<u32>(std::round(1000.0f / smoothed_frametime));
    smooth_frametime_avg.add_new_frametime(frametime);
  }

  if constexpr (multithreaded_engine) { 
    draw_fut.get();
    update_fut.get();
  }

  app_is_running = false;
  shutdown();
}

void Athi_Core::draw(GLFWwindow *window) {
  profile p("Athi_Core::draw");

  const f64 time_start_frame = glfwGetTime();
  glClearColor(background_color.r, background_color.g,
               background_color.b, background_color.a);
  check_gl_error();

  glClear(GL_COLOR_BUFFER_BIT);
  check_gl_error();

  if (post_processing) {
    profile p("post processing");

    framebuffers[0].clear();


    // First draw the particles to the framebuffer.
    framebuffers[0].bind();

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    check_gl_error();
    particle_system.draw();

    draw_fullscreen_quad(framebuffers[0].texture, vec2(0, 0));


    // .. Then blur the current framebuffer
    for (s32 i = 0; i < post_processing_samples; i++) {
      draw_fullscreen_quad(framebuffers[0].texture, vec2(0, 1 * blur_strength));
      draw_fullscreen_quad(framebuffers[0].texture, vec2(1 * blur_strength, 0));
    }
  }

  if (post_processing) {
    framebuffers[0].unbind();
    draw_fullscreen_quad(framebuffers[0].texture, vec2(0, 0));
  }


  if (draw_particles) particle_system.draw();
  if (draw_rects) render_rects();
  if (draw_lines) render_lines();


  render(); render_clear();


  //@Bug: rects and lines are being drawn over the Gui.
  if (show_settings) {
    update_settings();
    draw_custom_gui();
    gui_render();
  }
  

  {
    profile p("glfwSwapBuffers");
    glfwSwapBuffers(window);
  }
  render_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  render_framerate =
      static_cast<u32>(std::round(1000.0f / smoothed_render_frametime));
  smooth_render_rametime_avg.add_new_frametime(render_frametime);
}

void Athi_Core::update() {

  physics_profile p("update");

  const f64 time_start_frame = glfwGetTime();

  if (!particle_system.particles.empty()) {
    particle_system.update();
  }

  if (use_gravitational_force) {
    profile p("ParticleSystem::apply_n_body()");
    particle_system.apply_n_body();
  }

  if (cycle_particle_color)
    circle_color = color_over_time(sinf(glfwGetTime()* 0.5));

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

void Athi_Core::draw_loop()
{
  auto window_context = window.get_window_context();
  glfwMakeContextCurrent(window_context);
  while (app_is_running)
  {
    if constexpr (multithreaded_engine) update_inputs();
    draw(window_context);
  }
}

void Athi_Core::physics_loop()
{
  while (app_is_running)
  {
    update();
  }
}

void Athi_Core::update_settings() { glfwSwapInterval(vsync); }

void Athi_Core::shutdown() {
  gui_shutdown();
  glfwTerminate();
  save_variables();
  particle_system.save_state();
}
