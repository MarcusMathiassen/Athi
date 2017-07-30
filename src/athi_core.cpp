#define STB_IMAGE_IMPLEMENTATION
#include "athi_core.h"
#include "athi_utility.h"
#include "athi_slider.h"


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

  text_manager = std::make_unique<Athi_Text_Manager>();
  text_manager->font_atlas_path = "./res/font_custom.png";
  text_manager->init();

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
  add_text_dynamic("Framerate limit: ", &framerate_limit, LEFT, BOTTOM+ROW, "framerate_limit");
  add_text_dynamic("Frametime:       ", &frametime, LEFT, BOTTOM, "frametime");

  while (app_is_running)
  {
    f64 time_start_frame{ glfwGetTime() };
    glClear(GL_COLOR_BUFFER_BIT);

    if (show_settings)
    {
      update_UI();
      draw_UI();
      update_settings();
    }

    glfwSwapBuffers(window->get_window_context());

    if (framerate_limit != 0) limit_FPS(framerate_limit, time_start_frame);
    frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  }
}

template <typename T>
void Athi_Core::add_text_dynamic(string static_str, T* dynamic_str, f32 x, f32 y, string id)
{
  Athi_Text text;
  text.id = id;
  text.pos.x = x;
  text.pos.y = y;
  text.str = static_str;
  if constexpr (std::is_floating_point<T>::value) text.float_dynamic_part = dynamic_str;
  if constexpr (std::is_integral<T>::value) text.int_dynamic_part = dynamic_str;
  text_manager->text_buffer.emplace_back(std::make_unique<Athi_Text>(text));
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
  text_manager->draw();
}

void Athi_Core::shutdown()
{
  glfwTerminate();
}

