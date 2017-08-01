#define STB_IMAGE_IMPLEMENTATION
#include "athi_core.h"
#include "athi_utility.h"
#include "athi_slider.h"
#include "athi_checkbox.h"
#include "athi_input.h"
#include "athi_text.h"
#include "athi_circle.h"
#include "athi_camera.h"


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

  while (!glfwWindowShouldClose(window->get_window_context()))
  {
    window->update();
    glfwWaitEvents();
  }
  app_is_running = false;
  draw_thread.join();

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

  Athi_Text cpu_info_text;
  cpu_info_text.pos = vec2(LEFT+ROW, TOP);
  cpu_info_text.str = cpu_brand + " | " + std::to_string(cpu_cores) + " cores | " + std::to_string(cpu_threads) + " threads";
  add_text(&cpu_info_text);

  Athi_Text frametime_text;
  frametime_text.pos = vec2(LEFT+ROW, TOP-ROW);
  add_text(&frametime_text);


  Athi_Slider<s32> slider;
  slider.str = "FPS limit: ";
  slider.var = &framerate_limit;
  slider.pos = vec2(LEFT+ROW, TOP-ROW*2.5f);
  slider.min = 0;
  slider.max = 300;
  add_slider<s32>(&slider);

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

    update_inputs();

    update_circles();
    draw_circles();

    if (show_settings)
    {
      frametime_text.str = "FPS: " + std::to_string((u32)(std::round((1000.0f/smoothed_frametime)))) + " | Frametime: " + std::to_string(smoothed_frametime);
      circle_info.str = "Circles: " + std::to_string(get_num_circles());
      update_UI();
      draw_UI();
      update_settings();
    }

    glfwSwapBuffers(window->get_window_context());

    if (framerate_limit != 0) limit_FPS(framerate_limit, time_start_frame);
    frametime = (glfwGetTime() - time_start_frame) * 1000.0;

    smooth_frametime_avg.add_new_frametime(frametime);
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

