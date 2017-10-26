#pragma once

#include "athi_settings.h"
#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include <array>
#include <string>
#include <sstream>
#include <iostream>

static int optimizer_used{2};
static bool open_settings = false;
static bool open_profiler = false;

void gui_init(GLFWwindow *window, float px_scale);
void gui_shutdown();
void gui_render();

static void new_style();
static void SetupImGuiStyle(bool bStyleDark_, float alpha_);
static void menu_profiler();
static void menu_settings();

static void menu_profiler() {

  ImGui::Begin("Profiler");
  ImGui::Columns(3, "mycolumns");
  ImGui::Separator();
  ImGui::Text("Component");
  ImGui::NextColumn();
  ImGui::Text("Time (ms)");
  ImGui::NextColumn();
  ImGui::Text("%% of total");
  ImGui::NextColumn();

  ImGui::Separator();

  const std::array<std::string, 3> components{{"Total", "Render", "Update"}};
  const std::array<double, 3> times{{smoothed_frametime,
                                     smoothed_render_frametime,
                                     smoothed_physics_frametime}};

  const std::array<double, 3> of_total{
      {100.0 * smoothed_frametime / smoothed_frametime,
       100.0 * smoothed_render_frametime / smoothed_frametime,
       100.0 * smoothed_physics_frametime / smoothed_frametime}};

  const auto col = ImVec4(1.0f, 0.5f, 1.0f, 1.0f);
  for (int i = 0; i < 3; i++) {
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::Text("%s", components[i].c_str());
    ImGui::NextColumn();
    ImGui::PopStyleColor();
    ImGui::Text("%f", times[i]);
    ImGui::NextColumn();
    ImGui::Text("%.3f", of_total[i]);
    ImGui::NextColumn();
  }
  ImGui::Columns(1);
  ImGui::Separator();

  ImGui::End();
}

static void menu_settings() {

  ImGui::Begin("Settings", NULL,  ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::InputInt("Physics samples", &physics_samples);
  if (physics_samples < 1) physics_samples = 1;

  ImGui::Checkbox("VSync", &vsync); ImGui::SameLine();
  ImGui::Checkbox("Collision", &circle_collision); ImGui::SameLine();
  ImGui::Checkbox("OpenCL", &openCL_active);
  ImGui::SliderFloat("time scale", &slow_amount, 0.0001f, 10.0f);
  
  ImGui::Checkbox("gravity", &physics_gravity); ImGui::SameLine();
  ImGui::SliderFloat(" ", &gravity_force, 0.01f, 20.0f);
  
  ImGui::Checkbox("multithreaded", &use_multithreading); ImGui::SameLine();
  ImGui::InputInt("", &variable_thread_count);
  if (variable_thread_count < 0) variable_thread_count = 0;
  ImGui::Checkbox("draw nodes ", &draw_nodes); ImGui::SameLine();
  ImGui::Checkbox("color particles based on node", &color_particles);


  if (ImGui::CollapsingHeader("quadtree options"))
  {
    ImGui::Checkbox("Occupied only", &quadtree_show_only_occupied);
    ImGui::SliderInt("depth", &quadtree_depth, 0, 10);
    ImGui::SliderInt("capacity", &quadtree_capacity, 0, 100);
  }
  if (ImGui::CollapsingHeader("uniform grid options"))
  { 
    ImGui::SliderInt("nodes", &voxelgrid_parts, 4, 1024);
  }


  if (ImGui::CollapsingHeader("color options"))
  {
    ImGui::PushItemWidth(100.0f);
    ImGui::Text("particle color");
    ImGui::SameLine();    
    if (ImGui::Button("apply to all")) {
      for (auto& p: particle_manager.particles)
        particle_manager.colors[p.id] = circle_color;
    }
    ImGui::SameLine(200);    
    ImGui::Text("background color");

    ImGui::ColorPicker4("##particle", (float *)&circle_color);
    ImGui::SameLine();    
    ImGui::ColorPicker4("##Background", (float *)&background_color);
    
    ImGui::Text("quadtree colors");
    ImGui::Text("sw");
    ImGui::SameLine(200);    
    ImGui::Text("se");
    ImGui::SameLine(400);    
    ImGui::Text("nw");
    ImGui::SameLine(600);    
    ImGui::Text("ne");
    
    ImGui::ColorPicker4("##sw", (float *)&sw_color);
    ImGui::SameLine();
    ImGui::ColorPicker4("##se", (float *)&se_color);
    ImGui::SameLine();
    ImGui::ColorPicker4("##nw", (float *)&nw_color);
    ImGui::SameLine();
    ImGui::ColorPicker4("##ne", (float *)&ne_color);

    ImGui::PopItemWidth();
  }
  ImGui::End();
}

void gui_render() {
  ImGui_ImplGlfwGL3_NewFrame();

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("menu")) {
      ImGui::MenuItem("settings", NULL, &open_settings);
      ImGui::MenuItem("profiler", NULL, &open_profiler);
      ImGui::EndMenu();
    }

    const auto yellow = ImVec4(0.1f, 8.0f, 0.8f, 1.0f);    
    ImGui::PushStyleColor(ImGuiCol_Text, yellow);
    ImGui::Text("particles: %lu", particle_manager.particles.size());
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::Text("comparisons: %llu", static_cast<uint64_t>(comparisons) / physics_samples);
    ImGui::SameLine();
    ImGui::Text("resolution: %llu (%.4f%%)", static_cast<uint64_t>(resolutions), 100.0f * static_cast<float>(resolutions)/static_cast<float>(comparisons)); ImGui::SameLine();

    const auto red = ImVec4(1.0f, 0.1, 0.1f, 1.0f);
    const auto green = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, (framerate >= 60) ? green : red);
    ImGui::Text("FPS %d", framerate);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushItemWidth(100.0f);    
    ImGui::SliderFloat("particle size", &circle_size, 1.0f, 100.0f); ImGui::SameLine();     
    ImGui::SliderFloat("mouse size", &mouse_size, 1.0f, 500.0f); ImGui::SameLine();
    ImGui::PopItemWidth();    
    ImGui::Checkbox("grab", &mouse_grab); ImGui::SameLine();

    if (quadtree_active)
      optimizer_used = 0;
    else if (voxelgrid_active)
      optimizer_used = 1;
    else
      optimizer_used = 2;

    ImGui::RadioButton("quadtree", &optimizer_used, 0);   ImGui::SameLine();      
    ImGui::RadioButton("uniform grid", &optimizer_used, 1);   ImGui::SameLine();      
    ImGui::RadioButton("none", &optimizer_used, 2);
    
      switch (optimizer_used) {
      case 0:
        quadtree_active = true;
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
      default:
        break;
      }
    
    ImGui::EndMainMenuBar();
  }

  if (open_settings) menu_settings();
  if (open_profiler) menu_profiler();

  ImGui::Render();
}

void gui_init(GLFWwindow *window, float px_scale) {
  ImGui_ImplGlfwGL3_Init(window, false);
  ImGuiIO &io = ImGui::GetIO();
#ifdef __WINDOWS__
#else
  io.FontGlobalScale = 1.0f / px_scale;
  io.Fonts->AddFontFromFileTTF("../Resources/DroidSans.ttf",
            12 * px_scale, NULL,
            io.Fonts->GetGlyphRangesJapanese());
#endif
  //SetupImGuiStyle(true, 0.9f);
  new_style();
}

void gui_shutdown() {
  ImGui_ImplGlfwGL3_Shutdown();
}

static void new_style() {
  ImGuiStyle * style = &ImGui::GetStyle();
  
  style->WindowPadding = ImVec2(15, 15);
  style->WindowRounding = 5.0f;
  style->FramePadding = ImVec2(5, 5);
  style->FrameRounding = 4.0f;
  style->ItemSpacing = ImVec2(12, 8);
  style->ItemInnerSpacing = ImVec2(8, 6);
  style->IndentSpacing = 25.0f;
  style->ScrollbarSize = 15.0f;
  style->ScrollbarRounding = 9.0f;
  style->GrabMinSize = 5.0f;
  style->GrabRounding = 3.0f;

  style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
  style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
  style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
  style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
  style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
  style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
  style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
  style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
  style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
  style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
  style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
  style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
  style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
  style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
  style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
  style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
  style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
  style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
  style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
  style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
  style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
  style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

static void SetupImGuiStyle(bool bStyleDark_, float alpha_) {
  ImGuiStyle &style = ImGui::GetStyle();

  // light style from Pacôme Danhiez (user itamago)
  // https://github.com/ocornut/imgui/pull/511#issuecomment-175719267

  ImVec4 imgui_pastel_red(1.0f, 0.411f, 0.380f, 1.0f);
  style.Alpha = 1.0f;
  style.FrameRounding = 0.0f;
  style.WindowRounding = 0.0f;
  style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
  style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
  style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
  style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
  style.Colors[ImGuiCol_ScrollbarGrab] = imgui_pastel_red;
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
  style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_SliderGrab] = imgui_pastel_red;
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Header] = imgui_pastel_red;
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip] = imgui_pastel_red;
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  style.Colors[ImGuiCol_CloseButton] = imgui_pastel_red;
  style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
  style.Colors[ImGuiCol_PlotLines] = imgui_pastel_red;
  style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_TextSelectedBg] = imgui_pastel_red;
  style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

  if (bStyleDark_) {
    for (int i = 0; i <= ImGuiCol_COUNT; i++) {
      ImVec4 &col = style.Colors[i];
      float H, S, V;
      ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);
      if (S < 0.1f)
        V = 1.0f - V;
      ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
      if (col.w < 1.00f)
        col.w *= alpha_;
    }
  } else {
    for (int i = 0; i <= ImGuiCol_COUNT; i++) {
      ImVec4 &col = style.Colors[i];
      if (col.w < 1.00f) {
        col.x *= alpha_;
        col.y *= alpha_;
        col.z *= alpha_;
        col.w *= alpha_;
      }
    }
  }
}