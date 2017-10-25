#define STB_IMAGE_IMPLEMENTATION

#include "athi_core.h"
#include "athi_camera.h"
#include "athi_input.h"
#include "athi_line.h"
#include "athi_rect.h"
#include "athi_renderer.h"
#include "athi_settings.h"
#include "athi_spring.h"
#include "athi_utility.h"

#include <array>
#include <dispatch/dispatch.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "spdlog/spdlog.h"

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

Smooth_Average<double, 30> smooth_frametime_avg(&smoothed_frametime);
Smooth_Average<double, 30> smooth_physics_rametime_avg(&smoothed_physics_frametime);
Smooth_Average<double, 30> smooth_render_rametime_avg(&smoothed_render_frametime);

void SetupImGuiStyle( bool bStyleDark_, float alpha_  )
{
    ImGuiStyle& style = ImGui::GetStyle();
    
    // light style from Pacôme Danhiez (user itamago) https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
    style.Alpha = 1.0f;
    style.FrameRounding = 3.0f;
    style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
    style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_Column]                = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
    style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
    style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

    if( bStyleDark_ )
    {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            float H, S, V;
            ImGui::ColorConvertRGBtoHSV( col.x, col.y, col.z, H, S, V );

            if( S < 0.1f )
            {
               V = 1.0f - V;
            }
            ImGui::ColorConvertHSVtoRGB( H, S, V, col.x, col.y, col.z );
            if( col.w < 1.00f )
            {
                col.w *= alpha_;
            }
        }
    }
    else
    {
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            if( col.w < 1.00f )
            {
                col.x *= alpha_;
                col.y *= alpha_;
                col.z *= alpha_;
                col.w *= alpha_;
            }
        }
    }
}

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

  int width, height;
  glfwGetFramebufferSize(window.get_window_context(), &width, &height);
  const float font_retina_scale = static_cast<float>(width) / static_cast<float>(window.scene.width);
  ImGui_ImplGlfwGL3_Init(window.get_window_context(), false);
  ImGuiIO& io = ImGui::GetIO();
  io.FontGlobalScale = 1.0f / font_retina_scale;
  io.Fonts->AddFontFromFileTTF("../Resources/Andale Mono.ttf", 12 * font_retina_scale, NULL, io.Fonts->GetGlyphRangesJapanese());
  SetupImGuiStyle(true, 0.6f);

  auto console = spdlog::stdout_color_mt("Athi");
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
    framerate = static_cast<uint32_t>(std::round(1000.0f / smoothed_frametime));
    smooth_frametime_avg.add_new_frametime(frametime);
    comparisons = 0;
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

  if (show_settings) {
    ImGui_ImplGlfwGL3_NewFrame();    
    ImGui::Begin("Settings");
    ImGui::Text("Particles: %lu", particle_manager.particles.size());
    ImGui::Text("Comparisons: %d", static_cast<int32_t>(comparisons));
    ImGui::SliderFloat("Particle size", &circle_size, 1.0f, 100.0f);
    ImGui::SliderFloat("Mouse size", &mouse_size, 1.0f, 100.0f);ImGui::SameLine();
    ImGui::Checkbox("Grab", &mouse_grab);
    ImGui::Checkbox("VSync", &vsync);
    ImGui::Checkbox("Gravity", &physics_gravity);
    ImGui::Checkbox("Collision", &circle_collision);
    ImGui::Checkbox("OpenCL", &openCL_active);
    ImGui::Checkbox("Multithreaded", &use_multithreading); ImGui::SameLine(); ImGui::InputInt("", &variable_thread_count);
    ImGui::Checkbox("Debug", &draw_debug);ImGui::SameLine();
    ImGui::Checkbox("Nodes", &draw_nodes);ImGui::SameLine();
    ImGui::Checkbox("color particles", &color_particles);
    if (variable_thread_count < 0) variable_thread_count = 0;

    int prev_optimizer_used = optimizer_used;
    if (quadtree_active)optimizer_used = 0;
    else if (voxelgrid_active) optimizer_used = 1;
    else optimizer_used = 2;

    ImGui::Separator();
    ImGui::RadioButton("Quadtree", &optimizer_used, 0);ImGui::SameLine();
    ImGui::Checkbox("Occupied only", &quadtree_show_only_occupied);   
    ImGui::SliderInt("depth", &quadtree_depth, 0, 10);
    ImGui::SliderInt("capacity", &quadtree_capacity, 0, 100);
    ImGui::Separator();
    ImGui::RadioButton("Voxelgrid", &optimizer_used, 1);
    ImGui::SliderInt("nodes", &voxelgrid_parts, 0, 1024);
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

    ImGui::Separator();

    ImGui::Text("Profiler");
    ImGui::Columns(3, "mycolumns"); // 3-ways, with border
    ImGui::Separator();
    ImGui::Text("Component"); ImGui::NextColumn();
    ImGui::Text("Time (ms)"); ImGui::NextColumn();
    ImGui::Text("%% of total"); ImGui::NextColumn();

    ImGui::Separator();

    const std::array<std::string, 3> components 
    {{
      "Total", 
      "Render", 
      "Update"
    }};
    const std::array<double, 3> times 
    {{
      smoothed_frametime, 
      smoothed_render_frametime, 
      smoothed_physics_frametime
    }};

    const std::array<double, 3> of_total 
    {{
      100.0 * smoothed_frametime / smoothed_frametime, 
      100.0 * smoothed_render_frametime / smoothed_frametime, 
      100.0 * smoothed_physics_frametime / smoothed_frametime
    }};

    const auto col = ImVec4(1.0f,0.5f,1.0f,1.0f);
    for (int i = 0; i < 3; i++)
    {
      ImGui::PushStyleColor(ImGuiCol_Text, col);
      ImGui::Text("%s", components[i].c_str());   ImGui::NextColumn();
      ImGui::PopStyleColor();
        ImGui::Text("%f", times[i]);        ImGui::NextColumn(); 
        ImGui::Text("%.3f", of_total[i]);     ImGui::NextColumn(); 
    }
    ImGui::Columns(1);
    ImGui::Separator();

    ImGui::End();
    ImGui::Render();
  }

  render_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  render_framerate = static_cast<uint32_t>(std::round(1000.0f / smoothed_render_frametime));
  smooth_render_rametime_avg.add_new_frametime(render_frametime);

  glfwSwapBuffers(window);
}

void Athi_Core::update() {
  const double time_start_frame = glfwGetTime();
  int iter = 0;
  double timepassed = 0.0;
  while (timepassed < (1000.0 / 60.0)) {
    const double time_start_frame = glfwGetTime();    
    particle_manager.update();

  physics_frametime = (glfwGetTime() - time_start_frame) * 1000.0;
  physics_framerate = static_cast<uint32_t>(std::round(1000.0f / smoothed_physics_frametime));
  smooth_physics_rametime_avg.add_new_frametime(physics_frametime);
  timestep = smoothed_physics_frametime / (1000.0 / 60.0);
  timepassed += physics_frametime;
  }
}

void Athi_Core::update_settings() { glfwSwapInterval(vsync); }

void Athi_Core::shutdown() {   
  ImGui_ImplGlfwGL3_Shutdown();  
  glfwTerminate(); 
}
