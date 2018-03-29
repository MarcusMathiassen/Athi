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
#include "stb_image.h" // stbi_load, stbi_image_free

#include "./Utility/athi_save_state.h" // write_data, read_data
#include "./Utility/athi_config_parser.h" // init_variables
#include "./Renderer/athi_renderer.h" // render
#include "./Renderer/athi_text.h"// init_text_renderer
#include "./Renderer/opengl_utility.h" // check_gl_error();
#include "./Utility/athi_constant_globals.h" // os
#include "./Utility/profiler.h" // cpu_profile, gpu_profiler
#include "athi_gui.h" // gui_init, gui_render, gui_shutdown
#include "athi_input.h" // update_inputs
#include "./Renderer/athi_primitives.h" // draw_circle, draw_rects, draw_lines
#include "athi_settings.h" // console, ThreadPoolSolution

#include "athi_utility.h" // profile, Smooth_Average
#include "athi_window.h" // window
#include "athi_dispatch.h" // dispatch

#include "Utility/console.h" // console
#include "Utility/fixed_size_types.h" // u32, s32, etc.

#include <atomic> // atomic
#include <mutex> // mutex
#include <condition_variable> // condition_variable
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../dep/Universal/imgui.h"
#include "../dep/Universal/imgui_impl_glfw_gl3.h"
#include "../dep/Universal/spdlog/spdlog.h"

static Smooth_Average<f64, 30> smooth_frametime_avg(&smoothed_frametime);
static Smooth_Average<f64, 30> smooth_physics_rametime_avg(&smoothed_physics_frametime);
static Smooth_Average<f64, 30> smooth_render_rametime_avg(&smoothed_render_frametime);

static Renderer renderer;

static bool ready_to_draw{false};

static std::mutex draw_mutex;
static std::condition_variable can_draw_cond;

static void setup_fullscreen_quad()
{
  auto &shader = renderer.make_shader();
  shader.sources = {"athi_fullscreen_quad.vert", "athi_fullscreen_quad.frag"};
  shader.uniforms = {"res", "tex", "dir"};
  shader.preambles = {"blurs.glsl"};

  constexpr u16 indices[6] = {0, 1, 2, 0, 2, 3};
  auto &indices_buffer = renderer.make_buffer("indices");
  indices_buffer.data = (void*)indices;
  indices_buffer.data_size = sizeof(indices);
  indices_buffer.type = buffer_type::element_array;

  renderer.finish();
}

static void draw_fullscreen_quad(u32 texture, const vec2 &dir)
{
  CommandBuffer cmd;
  cmd.type = primitive::triangles;
  cmd.count = 6;
  cmd.has_indices = true;

  renderer.bind();

  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, texture);

  renderer.shader.set_uniform("tex", 0);
  renderer.shader.set_uniform("dir", dir);
  renderer.shader.set_uniform("res", framebuffer_width, framebuffer_height);

  renderer.draw(cmd);
}

static void guassian_blur(int samples, int strength)
{
  CommandBuffer cmd;
  cmd.type = primitive::triangles;
  cmd.count = 6;
  cmd.has_indices = true;

  renderer.bind();

  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, framebuffers[0].texture);

  renderer.shader.set_uniform("res", framebuffer_width, framebuffer_height);
  renderer.shader.set_uniform("tex", 0);

  for (s32 i = 0; i < samples; i++)
  {
    renderer.shader.set_uniform("dir", vec2(0, 1 * strength));
    renderer.draw(cmd);
    renderer.shader.set_uniform("dir", vec2(1 * strength, 0));
    renderer.draw(cmd);
  }
}

void Athi_Core::init()
{
  spdlog::set_pattern("[%H:%M:%S] %v");
  console = spdlog::stdout_color_mt("Athi");
  if constexpr (DEBUG_MODE) console->critical("DEBUG MODE: ON");
  if constexpr (multithreaded_engine) console->critical("MULTITHREADED ENGINE: ON");

  init_variables();

  init_window();

  stbi_set_flip_vertically_on_load(true);

  gui_init(get_window_context(), px_scale);
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

  init_text_renderer();

  init_circle_renderer();
  init_line_renderer();
  init_rect_renderer();
  init_input_manager();
  entity_manager.init();

  custom_gui_init();

  glClearColor(background_color.r, background_color.g,
               background_color.b, background_color.a);

  glEnable(GL_FRAMEBUFFER_SRGB);
  check_gl_error();
}

void Athi_Core::start()
{
  auto window_context = get_window_context();
  glfwMakeContextCurrent(window_context);

  setup_fullscreen_quad();

  framebuffers.resize(1);
  framebuffers[0].resize(framebuffer_width, framebuffer_height);

  std::future<void> update_fut;
  if constexpr (multithreaded_engine) {
    update_fut = dispatch.enqueue(&Athi_Core::physics_loop, this);
  }

  while (!glfwWindowShouldClose(window_context))
  {
    const auto time_start_frame = get_time();

    //@Hack @Apple: GLFW 3.3.0 has a bug that ignores vsync when not visible
    if (glfwGetWindowAttrib(window_context, GLFW_VISIBLE))
    {
      glfwPollEvents();
    } else {
      glfwWaitEvents();
    }

    if constexpr (DEBUG_MODE && !multithreaded_engine) { reload_variables(); }

    if constexpr (multithreaded_engine)
    {
      // GPU draw
      {
        std::unique_lock<std::mutex> lock(draw_mutex);
        can_draw_cond.wait(lock, []() { return ready_to_draw; });

        cpu_profile::clear_profiles();
        gpu_profile::clear_profiles();

        // Input
        update_inputs();

        // GPU draw
        draw(window_context);

        ready_to_draw = false;
        can_draw_cond.notify_one();
      }
    // Single threaded engine
    } else {

      cpu_profile::clear_profiles();
      gpu_profile::clear_profiles();

      // Input
      update_inputs();

      // CPU Update
      update(1.0f/60.0f);

      // GPU draw
      draw(window_context);
    }

    if (framerate_limit != 0) limit_FPS(framerate_limit, time_start_frame);
    frametime = (get_time() - time_start_frame) * 1000.0;
    framerate = static_cast<u32>(std::round(1000.0f / smoothed_frametime));
    smooth_frametime_avg.add_new_frametime(frametime);
  }

  app_is_running = false;

  if constexpr (multithreaded_engine) {
    update_fut.get();
  }

  shutdown();
}

void Athi_Core::draw(GLFWwindow *window)
{
  const auto time_start_frame = get_time();
  glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);
  check_gl_error();

  glClear(GL_COLOR_BUFFER_BIT);
  check_gl_error();

  // Upload gpu buffers
  particle_system.gpu_buffer_update();

  // @Hot: We seem to be fillrate limited.
  //  Igpus have a hard time at higher resolutions.
  if (post_processing)
  {
    gpu_profile p("post processing");

    framebuffers[0].clear();
    framebuffers[0].bind();
    //glDrawBuffer(GL_COLOR_ATTACHMENT0); check_gl_error();

    // Draw all objects that needs blur
    particle_system.draw();

    // .. Then blur the current framebuffer
    guassian_blur(post_processing_samples, blur_strength);

    // ... then draw to the main framebuffer
    framebuffers[0].unbind();
    draw_fullscreen_quad(framebuffers[0].texture, vec2(0, 0));
  }

  particle_system.draw_debug_nodes();

  // Draw entities
  entity_manager.draw();

  if (draw_particles)   particle_system.draw();
  if (draw_rects)       render_rects();
  if (draw_lines)       render_lines();
  if (draw_circles)     render_circles();

  render();

  //@Bug: rects and lines are being drawn over the Gui.
  draw_custom_gui();
  update_settings();

  if (show_settings)
  {
    gui_render();
  }

  {
    gpu_profile p("glfwSwapBuffers");
    glfwSwapBuffers(window);
  }

  render_frametime = (get_time() - time_start_frame) * 1000.0;
  render_framerate = static_cast<u32>(std::round(1000.0f / smoothed_render_frametime));
  smooth_render_rametime_avg.add_new_frametime(render_frametime);

}

void Athi_Core::update(float dt)
{
  const auto time_start_frame = get_time();

  // Update entities
  entity_manager.update(dt);

  // Update objects
  particle_system.update(dt);

  if (use_gravitational_force)
  {
    cpu_profile p("ParticleSystem::apply_n_body()");
    particle_system.apply_n_body();
  }

  if (cycle_particle_color)
    circle_color = color_over_time(get_time());

  // Update objects gpu data
  particle_system.update_data();

  // Update timers
  physics_frametime = (get_time() - time_start_frame) * 1000.0;
  physics_framerate = static_cast<u32>(std::round(1000.0f / smoothed_physics_frametime));
  smooth_physics_rametime_avg.add_new_frametime(physics_frametime);
  timestep = dt;
}

void Athi_Core::draw_loop()
{
  auto window_context = get_window_context();
  glfwMakeContextCurrent(window_context);
  while (app_is_running)
  {
    {
      std::unique_lock<std::mutex> lock(draw_mutex);
      can_draw_cond.wait(lock, []() { return ready_to_draw; });

      draw(window_context);
      ready_to_draw = false;
    }
    can_draw_cond.notify_one();
  }
}

void Athi_Core::physics_loop()
{
  while (app_is_running)
  {
    {
      std::unique_lock<std::mutex> lock(draw_mutex);
      can_draw_cond.wait(lock, []() { return !ready_to_draw; });
      update(1.0f/60.0f);
      ready_to_draw = true;
      can_draw_cond.notify_one();
    }
  }
}

void Athi_Core::update_settings() { glfwSwapInterval(vsync); }

void Athi_Core::shutdown() {
  save_variables();
  particle_system.save_state();
  gui_shutdown();
  glfwTerminate();
}
