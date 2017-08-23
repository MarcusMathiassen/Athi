#define STB_IMAGE_IMPLEMENTATION
#include "athi_core.h"
#include "athi_camera.h"
#include "athi_checkbox.h"
#include "athi_circle.h"
#include "athi_input.h"
#include "athi_quadtree.h"
#include "athi_rect.h"
#include "athi_settings.h"
#include "athi_slider.h"
#include "athi_text.h"
#include "athi_line.h"
#include "athi_ui_list.h"
#include "athi_utility.h"
#include "athi_softbody.h"
#include "athi_voxelgrid.h"
#include "athi_renderer.h"
#include "athi_spring.h"
#include <functional>

#include <iostream>
#include <thread>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

SMA smooth_frametime_avg(&smoothed_frametime);
SMA smooth_physics_rametime_avg(&smoothed_physics_frametime);

void Athi_Core::init()
{
  window = std::make_unique<Athi_Window>();
  window->scene.width = 512;
  window->scene.height = 512;
  window->init();

  init_input_manager();
  init_text_manager();
  init_rect_manager();
  init_circle_manager();
  init_line_manager();

  init_quadtree();
  init_voxelgrid();

  glEnable(GL_FRAMEBUFFER_SRGB);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_BUFFER);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(4 / 255.0f, 32 / 255.0f, 41 / 255.0f, 1);  

  cpu_threads = std::thread::hardware_concurrency();

  std::cout << "Status: " << glGetString(GL_VERSION) << '\n';
  std::cout << "Status: " << glGetString(GL_VENDOR) << '\n';
  std::cout << "Status: " << glGetString(GL_RENDERER) << '\n';

  std::cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << '\n';
  std::cout << "Status: Using GLFW " << glfwGetVersionString() << '\n';

  variable_thread_count = cpu_threads;
}

void Athi_Core::start()
{
  Athi_Checkbox quadtree_box;
  quadtree_box.pos = vec2(RIGHT - ROW * 7.5f, BOTTOM + ROW);
  quadtree_box.text.str = "quadtree";
  quadtree_box.variable = &quadtree_active;
  add_checkbox(&quadtree_box);

  Athi_Checkbox voxelgrid_box;
  voxelgrid_box.pos = vec2(RIGHT - ROW * 7.5f, BOTTOM + ROW * 2.0f);
  voxelgrid_box.text.str = "voxelgrid";
  voxelgrid_box.variable = &voxelgrid_active;
  add_checkbox(&voxelgrid_box);

  Athi_Checkbox gravity_box;
  gravity_box.pos = vec2(RIGHT - ROW * 7.5f, BOTTOM + ROW * 3.0f);
  gravity_box.text.str = "gravity";
  gravity_box.variable = &physics_gravity;
  add_checkbox(&gravity_box);

  Athi_Checkbox multithread_box;
  multithread_box.pos = vec2(RIGHT - ROW * 7.5f, BOTTOM + ROW * 4.0f);
  multithread_box.text.str = "multithreading";
  multithread_box.variable = &use_multithreading;
  add_checkbox(&multithread_box);

  Athi_Checkbox visualizer_box;
  visualizer_box.pos = vec2(RIGHT - ROW * 7.5f, BOTTOM + ROW * 5.0f);
  visualizer_box.text.str = "visualizer";
  visualizer_box.variable = &draw_debug;
  add_checkbox(&visualizer_box);

  Athi_Checkbox collisions_box;
  collisions_box.pos = vec2(RIGHT - ROW * 7.5f, BOTTOM + ROW * 6.0f);
  collisions_box.text.str = "circle collisons";
  collisions_box.variable = &circle_collision;
  add_checkbox(&collisions_box);

  Athi_Checkbox quadtree_show_filled;
  quadtree_show_filled.pos = vec2(RIGHT - ROW * 7.5f, BOTTOM + ROW * 7.0f);
  quadtree_show_filled.text.str = "quadtree show all";
  quadtree_show_filled.variable = &quadtree_show_only_occupied;
  add_checkbox(&quadtree_show_filled);

  Athi_Checkbox mouse_grab_multiple_box;
  mouse_grab_multiple_box.pos = vec2(RIGHT - ROW * 7.5f, BOTTOM + ROW * 8.0f);
  mouse_grab_multiple_box.text.str = "mouse grab multiple";
  mouse_grab_multiple_box.variable = &mouse_grab_multiple;
  add_checkbox(&mouse_grab_multiple_box);

  Athi_Checkbox vsync_box;
  vsync_box.pos = vec2(LEFT + ROW, TOP - ROW * 3.5f);
  vsync_box.active_if = &show_fps_info;
  vsync_box.text.str = "vsync";
  vsync_box.variable = &vsync;
  add_checkbox(&vsync_box);

  Athi_Slider<u32> physics_updates_per_sec_slider;
  physics_updates_per_sec_slider.str = "Physics FPS limit: ";
  physics_updates_per_sec_slider.var = &physics_FPS_limit;
  physics_updates_per_sec_slider.active_if = &show_fps_info;
  physics_updates_per_sec_slider.var_indicator = &physics_framerate;
  physics_updates_per_sec_slider.pos = vec2(LEFT + ROW, TOP - ROW * 5.5f);
  physics_updates_per_sec_slider.min = 0;
  physics_updates_per_sec_slider.max = 300;
  add_slider<u32>(&physics_updates_per_sec_slider);

  Athi_Slider<u32> framerate_limit_slider;
  framerate_limit_slider.str = "FPS limit: ";
  framerate_limit_slider.var = &framerate_limit;
  framerate_limit_slider.active_if = &show_fps_info;
  framerate_limit_slider.var_indicator = &framerate;
  framerate_limit_slider.pos = vec2(LEFT + ROW, TOP - ROW * 2.5f);
  framerate_limit_slider.min = 0;
  framerate_limit_slider.max = 300;
  add_slider<u32>(&framerate_limit_slider);

  Athi_Slider<f32> circle_size_slider;
  circle_size_slider.str = "Circle size: ";
  circle_size_slider.var = &circle_size;
  circle_size_slider.pos = vec2(LEFT + ROW, BOTTOM + ROW);
  circle_size_slider.min = 0.001f;
  circle_size_slider.max = 0.1f;
  add_slider<f32>(&circle_size_slider);

  Athi_Slider<u32> quadtree_depth_slider;
  quadtree_depth_slider.str = "Quadtree depth: ";
  quadtree_depth_slider.active_if = &quadtree_active;
  quadtree_depth_slider.var = &quadtree_depth;
  quadtree_depth_slider.var_indicator = nullptr;
  quadtree_depth_slider.pos = vec2(LEFT + ROW, TOP - ROW * 8.5f);
  quadtree_depth_slider.min = 0;
  quadtree_depth_slider.max = 10;
  add_slider<u32>(&quadtree_depth_slider);

  Athi_Slider<u32> quadtree_capacity_slider;
  quadtree_capacity_slider.str = "Quadtree capacity: ";
  quadtree_capacity_slider.active_if = &quadtree_active;
  quadtree_capacity_slider.var = &quadtree_capacity;
  quadtree_capacity_slider.var_indicator = nullptr;
  quadtree_capacity_slider.pos = vec2(LEFT + ROW, TOP - ROW * 11.5f);
  quadtree_capacity_slider.min = 0;
  quadtree_capacity_slider.max = 100;
  add_slider<u32>(&quadtree_capacity_slider);

  Athi_Slider<u32> multithreaded_collision_thread_count_slider;
  multithreaded_collision_thread_count_slider.str = "Threads: ";
  multithreaded_collision_thread_count_slider.var = &variable_thread_count;
  multithreaded_collision_thread_count_slider.active_if = &use_multithreading;
  multithreaded_collision_thread_count_slider.pos = vec2(RIGHT - ROW * 7.5f, TOP - ROW * 2.5f);
  multithreaded_collision_thread_count_slider.width = 0.3f;
  multithreaded_collision_thread_count_slider.min = 0;
  multithreaded_collision_thread_count_slider.max = cpu_threads*4;
  add_slider<u32>(&multithreaded_collision_thread_count_slider);

  Athi_Slider<f32> mouse_size_slider;
  mouse_size_slider.str = "Mouse box size: ";
  mouse_size_slider.var = &mouse_size;
  mouse_size_slider.active_if = &show_settings;
  mouse_size_slider.pos = vec2(RIGHT - ROW * 7.5f, TOP - ROW * 5.5f);
  mouse_size_slider.width = 0.3f;
  mouse_size_slider.min = 0.001f;
  mouse_size_slider.max = 1.0f;
  add_slider<f32>(&mouse_size_slider);

  auto window_context       = window->get_window_context();
  auto monitor_refreshrate  = window->monitor_refreshrate;
 
  std::thread draw_thread(&Athi_Core::draw_loop, this);
  std::thread physics_thread(&Athi_Core::physics_loop, this);

  while (!glfwWindowShouldClose(window_context))
  {
    const f64 time_start_frame = glfwGetTime();
    update_inputs();

    window->update();

    if (show_settings)
    {
      update_UI();
      update_settings();
    }

    glfwPollEvents();
    limit_FPS(monitor_refreshrate, time_start_frame);
  }

  app_is_running = false;
  draw_thread.join();
  physics_thread.join();
  
  shutdown();
}

void Athi_Core::draw_loop()
{
  Athi_Text frametime_text;
  frametime_text.pos = vec2(LEFT + ROW, TOP);
  add_text(&frametime_text);

  Athi_Text physics_frametime_text;
  physics_frametime_text.pos = vec2(LEFT+ROW, TOP-ROW);
  add_text(&physics_frametime_text);

  Athi_Text circle_info_text;
  circle_info_text.pos = vec2(LEFT + ROW, BOTTOM + ROW * 3.0f);
  add_text(&circle_info_text);

  auto window_context = window->get_window_context();
  glfwMakeContextCurrent(window_context);

  while (app_is_running)
  {
    const f64 time_start_frame = glfwGetTime();
    frametime_text.str         = "Render:  " + std::to_string(framerate) + "fps | " + std::to_string(smoothed_frametime) + "ms";
    physics_frametime_text.str = "Physics: " + std::to_string(physics_framerate) + "fps | " + std::to_string(smoothed_physics_frametime) + "ms";
    circle_info_text.str       = "Circles: " + std::to_string(get_num_circles());

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_circles();
    draw_rects();
    draw_lines();
    draw_springs();

    if (show_settings) draw_UI();

    render();
    glfwSwapBuffers(window_context);

    if (framerate_limit != 0) limit_FPS(framerate_limit, time_start_frame);
    frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    framerate = (u32)(std::round(1000.0f / smoothed_frametime));
    smooth_frametime_avg.add_new_frametime(frametime);
  }
}

void update_game_state()
{
  update_circles();
}

void Athi_Core::physics_loop()
{
  while (app_is_running)
  {
    const f64 time_start_frame = glfwGetTime();

    update_game_state();

    if (physics_FPS_limit != 0) limit_FPS(physics_FPS_limit, time_start_frame);
    physics_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    physics_framerate = (u32)(std::round(1000.0f / smoothed_physics_frametime));
    smooth_physics_rametime_avg.add_new_frametime(physics_frametime);
    timestep = smoothed_physics_frametime / (1000.0 / 60.0);
  }
}

void Athi_Core::update_settings()
{ 
  glfwSwapInterval(vsync); 
}

void Athi_Core::shutdown() 
{
  glfwTerminate(); 
}
