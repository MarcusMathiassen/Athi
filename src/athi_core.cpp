#define STB_IMAGE_IMPLEMENTATION
#include "athi_core.h"
#include "athi_utility.h"
#include "athi_slider.h"
#include "athi_checkbox.h"
#include "athi_input.h"
#include "athi_text.h"
#include "athi_circle.h"


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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0,0,0,1);

  ui_manager = std::make_unique<Athi_UI_Manager>();
  ui_manager->scale = 1.0f;

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

  Athi_Checkbox box;
  box.pos = vec2(LEFT+ROW*9,BOTTOM+ROW*2.5f);
  box.text.str = "vsync";
  box.width = 0.03f;
  box.height = 0.03f;
  box.variable = &vsync;
  box.init();

  Athi_Text frametime_text;
  frametime_text.pos = vec2(LEFT, BOTTOM);
  add_text(&frametime_text);

  Athi_Text frame_limit_text;
  frame_limit_text.pos = vec2(LEFT, BOTTOM+ROW);
  add_text(&frame_limit_text);

  Athi_Text cpu_info_text;
  cpu_info_text.pos = vec2(LEFT, TOP);
  cpu_info_text.str = cpu_brand + " | " + std::to_string(cpu_cores) + " cores | " + std::to_string(cpu_threads) + " threads";
  add_text(&cpu_info_text);

  SMA smooth_frametime_avg(&smoothed_frametime);

  while (app_is_running)
  {
    f64 time_start_frame{ glfwGetTime() };
    glClear(GL_COLOR_BUFFER_BIT);

    if (show_settings)
    {
      frametime_text.str = "FPS: " + std::to_string((u32)(1000/smoothed_frametime)) + " | Frametime: " + std::to_string(smoothed_frametime);
      frame_limit_text.str = "Framerate limit: " + std::to_string(framerate_limit);
      box.update();
      box.draw();
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


void Athi_Core::update_UI()
{
  ui_manager->update();
}

void Athi_Core::draw_UI()
{
  ui_manager->draw();
}

void Athi_Core::shutdown()
{
  glfwTerminate();
}

