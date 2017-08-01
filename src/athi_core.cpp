#define STB_IMAGE_IMPLEMENTATION
#include "athi_core.h"
#include "athi_utility.h"
#include "athi_slider.h"
#include "athi_checkbox.h"
#include "athi_input.h"
#include "athi_text.h"
#include "athi_circle.h"
#include "athi_camera.h"
#include "athi_settings.h"

#include <thread>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

void Athi_Core::init()
{
  window = std::make_unique<Athi_Window>();
  window->scene.width = 512;
  window->scene.height = 512;
  window->init();

  init_input_manager();
  init_text_manager();
  init_circle_manager();

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(4/255.0f,32/255.0f,41/255.0f,1);

  cpu_cores   = get_cpu_cores();
  cpu_threads = get_cpu_threads();
  cpu_brand   = get_cpu_brand();
}

void Athi_Core::start()
{
  std::thread draw_thread(&Athi_Core::draw_loop, this);
  std::thread physics_thread(&Athi_Core::physics_loop, this);

  while (!glfwWindowShouldClose(window->get_window_context()))
  {
    window->update();
    //update_inputs();
    if (show_settings) update_settings();
    glfwWaitEvents();
  }
  app_is_running = false;
  draw_thread.join();
  physics_thread.join();

  shutdown();
}

void Athi_Core::draw_loop()
{
  glfwMakeContextCurrent(window->get_window_context());

  Athi_Checkbox vsync_box;
  vsync_box.pos = vec2(LEFT+ROW,TOP-ROW*3.5f);
  vsync_box.text.str = "vsync";
  vsync_box.variable = &vsync;
  add_checkbox(&vsync_box);

  Athi_Checkbox quadtree_box;
  quadtree_box.pos = vec2(RIGHT-ROW*5.5f,BOTTOM+ROW);
  quadtree_box.text.str = "quadtree";
  quadtree_box.variable = &quadtree_active;
  add_checkbox(&quadtree_box);

  Athi_Checkbox gravity_box;
  gravity_box.pos = vec2(RIGHT-ROW*5.5f,BOTTOM+ROW*2);
  gravity_box.text.str = "gravity";
  gravity_box.variable = &physics_gravity;
  add_checkbox(&gravity_box);

  Athi_Checkbox multithread_box;
  multithread_box.pos = vec2(RIGHT-ROW*5.5f,BOTTOM+ROW*3);
  multithread_box.text.str = "multithreading";
  multithread_box.variable = &use_multithreading;
  add_checkbox(&multithread_box);

  Athi_Text cpu_info_text;
  cpu_info_text.pos = vec2(LEFT+ROW, TOP);
  cpu_info_text.str = cpu_brand + " | " + std::to_string(cpu_cores) + " cores | " + std::to_string(cpu_threads) + " threads";
  add_text(&cpu_info_text);

  Athi_Text frametime_text;
  frametime_text.pos = vec2(LEFT+ROW, TOP-ROW);
  add_text(&frametime_text);

  Athi_Slider<u32> physics_updates_per_sec_slider;
  physics_updates_per_sec_slider.str = "Physics updates per sec: ";
  physics_updates_per_sec_slider.var = &physics_updates_per_sec;
  physics_updates_per_sec_slider.var_indicator = &physics_framerate;
  physics_updates_per_sec_slider.pos = vec2(LEFT+ROW, TOP-ROW*5.5f);
  physics_updates_per_sec_slider.min = 0;
  physics_updates_per_sec_slider.max = 300;
  add_slider<u32>(&physics_updates_per_sec_slider);

  Athi_Slider<u32> framerate_limit_slider;
  framerate_limit_slider.str = "FPS limit: ";
  framerate_limit_slider.var = &framerate_limit;
  framerate_limit_slider.var_indicator = &framerate;
  framerate_limit_slider.pos = vec2(LEFT+ROW, TOP-ROW*2.5f);
  framerate_limit_slider.min = 0;
  framerate_limit_slider.max = 300;
  add_slider<u32>(&framerate_limit_slider);

  Athi_Text circle_info;
  circle_info.pos = vec2(LEFT+ROW, BOTTOM+ROW*2.5f);
  add_text(&circle_info);

  Athi_Slider<f32> circle_size_slider;
  circle_size_slider.str = "Circle size: ";
  circle_size_slider.var = &circle_size;
  circle_size_slider.pos = vec2(LEFT+ROW, BOTTOM+ROW);
  circle_size_slider.min = 0.001f;
  circle_size_slider.max = 0.1f;
  add_slider<f32>(&circle_size_slider);

  SMA smooth_frametime_avg(&smoothed_frametime);

  while (app_is_running)
  {
    f64 time_start_frame{ glfwGetTime() };
    glClear(GL_COLOR_BUFFER_BIT);

    // !*!*"#!*"#*!"#*!"*#*!"#*!*"#*!"#*!*"#**"#*!*!"
    draw_circles();
    update_inputs(); // this should be played in the window thread.
    // but currently that draws a bugged circle when pressing 1 covering the screen.
    // The bug happens when the draw thread updates faster than the physics thread.


    if (show_settings)
    {
      frametime_text.str = "FPS: " + std::to_string(framerate) + " | Frametime: " + std::to_string(smoothed_frametime) + " | Physics frametime: " + std::to_string(physics_frametime);
      circle_info.str = "Circles: " + std::to_string(get_num_circles());
      update_UI();
      draw_UI();
    }

    glfwSwapBuffers(window->get_window_context());

    if (framerate_limit != 0) limit_FPS(framerate_limit, time_start_frame);
    frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    framerate = (u32)(std::round(1000.0f/smoothed_frametime));
    smooth_frametime_avg.add_new_frametime(frametime);
  }
}

// smoothed_physics_frametime IS BUGGY ATM
void Athi_Core::physics_loop()
{
  SMA smooth_physics_rametime_avg(&smoothed_physics_frametime);
  while (app_is_running)
  {
    f64 time_start_frame{ glfwGetTime() };

    update_circles();

    if (physics_updates_per_sec != 0) limit_FPS(physics_updates_per_sec, time_start_frame);
    physics_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
    physics_framerate = (u32)(std::round(1000.0f/physics_frametime));
    smooth_physics_rametime_avg.add_new_frametime(physics_frametime);
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

