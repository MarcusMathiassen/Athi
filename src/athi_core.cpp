#include "athi_core.h"
#include "athi_utility.h"

#include <thread>
#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

void Athi_Core::init()
{
  window    = std::make_unique<Athi_Window>();
  renderer  = std::make_unique<Athi_Core_Renderer>();

  window->init();
  renderer->init();
  renderer->framerate_limit = 144;

  ImGui_ImplGlfwGL3_Init(window->get_window_context(), true);
  ImGuiIO& io = ImGui::GetIO();
  io.Fonts->AddFontFromFileTTF("./res/DroidSans.ttf", 14.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
  auto& style = ImGui::GetStyle();
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.90f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.0f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.90f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1f, 0.1f, 0.1f, 0.90f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.3f, 0.3f, 0.3f, 0.90f);
  style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1f, 0.1f, 0.1f, 0.0f);

  cpu_cores   = get_cpu_cores();
  cpu_threads = get_cpu_threads();
  cpu_brand   = get_cpu_brand();
}

void Athi_Core::start()
{
  std::thread rendering_thread(&Athi_Core::draw_loop, this);

  while (!glfwWindowShouldClose(window->get_window_context()))
  {
    window->update();
    glfwWaitEvents();
  }
  app_is_running = false;
  rendering_thread.join();

  shutdown();
}

void Athi_Core::draw_loop()
{
  glfwMakeContextCurrent(window->get_window_context());

  while (app_is_running)
  {
    f64 time_start_frame{ glfwGetTime() };
    ImGui_ImplGlfwGL3_NewFrame();
    glClear(GL_COLOR_BUFFER_BIT);

    renderer->update();

    ImGui::Begin("Settings", &show_settings, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    UI();
    renderer->UI();
    ImGui::End();

    if (show_settings) ImGui::Render();

    glfwSwapBuffers(window->get_window_context());
    if (renderer->framerate_limit != 0) limit_FPS(renderer->framerate_limit, time_start_frame);

    renderer->frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  }
}

void Athi_Core::UI()
{
  ImGui::Text("%s", cpu_brand.c_str());
  ImGui::Text("%d cores | %d threads", cpu_cores, cpu_threads);
  ImGui::Separator();
}

void Athi_Core::shutdown()
{
  ImGui_ImplGlfwGL3_Shutdown();
  glfwTerminate();
}
