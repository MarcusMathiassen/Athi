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


#pragma once

#include "athi_typedefs.h"

#include "athi_particle.h" // particle_system
#include "athi_settings.h" // has_random_velocity, etc.
#include "./Renderer/athi_primitives.h" // draw_line, draw_rect, draw_circle
#include "./Renderer/athi_camera.h" // camera
#include "./Renderer/athi_text.h" // draw_text
#include "athi_input.h" // mouse_pos
#include "./Utility/profiler.h" // cpu_profile, gpu_profile


#include "../dep/Universal/imgui.h"
#include "../dep/Universal/imgui_internal.h"
#include "../dep/Universal/imgui_impl_glfw_gl3.h"

static bool open_settings = false;
static bool open_gpu_profiler = false;
static bool open_cpu_profiler = false;
static bool open_debug_menu = false;

static u32 my_font;
static u32 ortho_loc;

static bool show_benchmark_menu = false;

void gui_init(GLFWwindow *window, float px_scale);
void gui_shutdown();
void gui_render();

// Custom GUI by Marcus
static int button_count = 0;
static float button_original_start_pos_x = 50.0f;
static float button_start_pos_x = 50.0f;

static bool button(
  const string& text,
  const vec4& color = pastel_gray,
  const vec4& selected_color = pastel_red,
  const vec4& pressed_color = pastel_green
  )
{
  vec2 button_pos{button_start_pos_x, 50.0f + gButtonHeight*2.0f*button_count++};

  int char_count = static_cast<s32>(text.length());
  if (char_count == 0) char_count = 4;
  float button_width = 17.9f * char_count;
  float button_height = 25.0f;
  float stack_margin = button_height * 2.0f;

  bool mouse_inside = false;
  bool button_hold = false;
  bool button_pressed = false;

  const auto mouse_pos = athi_input_manager.mouse.pos;
  Rect box;
  box.min.x = button_pos.x;
  box.min.y = button_pos.y;
  box.max.x = button_pos.x + button_width;
  box.max.y = button_pos.y + button_height;

  string button_text = text;
  vec4 button_color = color;

  // If mouse is inside
  if (box.contains(mouse_pos, 0.01f))
  {
    button_color = selected_color;
    mouse_inside = true;

    if (get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      button_hold = true;
      button_color = pressed_color;
    }
    if (get_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
      button_pressed = true;
    }
  }

  // Draw text inside this rect
  //draw_rounded_rect({button_pos.x, button_pos.y},  button_width, button_height, button_color, false);

  immidiate_draw_text(my_font, button_text, button_pos.x, button_pos.y, 1.0f * button_height * 0.1/4.0, white);

  return button_pressed;
}

static void draw_custom_gui() noexcept
{
  button_count = 0;

  if (button("GPU: " + std::to_string(smoothed_render_frametime) + "ms"))
  {
    //open_gpu_profiler ^= 1;
  }

  if (button("CPU: " + std::to_string(smoothed_physics_frametime) + "ms"))
  {
    //open_cpu_profiler ^= 1;
  }

  if (button("FPS: " + std::to_string(framerate), (framerate < 60) ? pastel_red : pastel_green))
  {
    //vsync ^= 1;
  }

  if (button("Particles: " + std::to_string(particle_system.particle_count)))
  {
  }
}

static void custom_gui_init() noexcept
{
    //init_text_renderer();
    my_font = load_font("DroidSans.ttf", 24*2);
}


static void new_style();
static void SetupImGuiStyle(bool bStyleDark_, float alpha_);
static void menu_gpu_profiler();
static void menu_settings();

static void ToggleButton(const char* str_id, bool* v)
{
  ImVec2 p = ImGui::GetCursorScreenPos();
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  float height = ImGui::GetFrameHeight() * 0.7f;
  float width = height * 1.55f;
  float radius = height * 0.50f;

  if (ImGui::InvisibleButton(str_id, ImVec2(width, height)))
      *v = !*v;
  ImU32 col_bg;
  if (ImGui::IsItemHovered())
      col_bg = *v ? IM_COL32(145+20, 211, 68+20, 255) : IM_COL32(218-20, 218-20, 218-20, 255);
  else
      col_bg = *v ? IM_COL32(145, 211, 68, 255) : IM_COL32(218, 218, 218, 255);

  draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
  draw_list->AddCircleFilled(ImVec2(*v ? (p.x + width - radius) : (p.x + radius), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
  ImGui::SameLine();
  ImGui::Text("%s",str_id);
}

static void menu_debug() {
  //ImGui::Begin("Debug Options", NULL, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::SetNextWindowSizeConstraints(ImVec2(-1, 0),    ImVec2(-1, FLT_MAX));
  ImGui::Begin("Debug Options");

  ImGui::SliderFloat("gButtonWidth", &gButtonWidth,  0, 500);
  ImGui::SliderFloat("gButtonHeight", &gButtonHeight,  0, 500);


  ToggleButton("cycle_particle_color", &cycle_particle_color);
  ToggleButton("draw_rects", &draw_rects);
  ToggleButton("draw_circles", &draw_circles);
  ToggleButton("draw_lines", &draw_lines);
  ToggleButton("wireframe_mode", &wireframe_mode);
  ToggleButton("has_random_velocity", &has_random_velocity);
  ToggleButton("draw_particles", &draw_particles);
  ToggleButton("is_particles_colored_by_acc", &is_particles_colored_by_acc);
  ToggleButton("tree_optimized_size", &tree_optimized_size);
  ToggleButton("mouse_grab", &mouse_grab);
  ToggleButton("show_mouse_collision_box", &show_mouse_collision_box);
  ToggleButton("show_mouse_grab_lines", &show_mouse_grab_lines);
  ToggleButton("draw_debug", &draw_debug);
  ToggleButton("color_particles", &color_particles);
  ToggleButton("draw_nodes", &draw_nodes);
  ToggleButton("quadtree_show_only_occupied", &quadtree_show_only_occupied);
  ToggleButton("quadtree_active", &quadtree_active);
  ToggleButton("use_uniformgrid", &use_uniformgrid);
  ToggleButton("vsync", &vsync);
  ToggleButton("use_gravitational_force", &use_gravitational_force);
  ToggleButton("use_multithreading", &use_multithreading);
  ToggleButton("use_libdispatch", &use_libdispatch);
  ToggleButton("openCL_active", &openCL_active);
  ToggleButton("post_processing", &post_processing);

  ImGui::End();
}

static void menu_cpu_profiler() {
  cpu_profile p("menu_cpu_profiler");

  ImGui::Begin("CPU profiler");

  ImGui::Text("Frametime: %.3f", smoothed_physics_frametime);

  ImGui::Columns(3, "mycolumns");
  ImGui::Separator();
  ImGui::Text("Function(s)");
  ImGui::NextColumn();
  ImGui::Text("Time (ms)");
  ImGui::NextColumn();
  ImGui::Text("%% of total");
  ImGui::NextColumn();

  ImGui::Separator();

  const auto col = ImVec4(0.5f, 1.0f, 0.8f, 1.0f);

  // if you want it sorted by time taken
  // auto new_map = flip_map(time_taken_by);

  for (const auto &[id, time] : cpu_profiles) {
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::Text("%s", id.c_str());
    ImGui::NextColumn();
    ImGui::PopStyleColor();

    ImGui::Text("%f", time);
    ImGui::NextColumn();

    auto perc = 100.0 * time / frametime;
    // If % is over 50% color red
    if (perc > 50) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.5f, 0.5f, 1.0f));
      ImGui::Text("%.3f", perc);
      ImGui::PopStyleColor();
    } else {
      ImGui::Text("%.3f", perc);
    }
    ImGui::NextColumn();
  }
  cpu_profiles.clear();

  ImGui::Columns(1);
  ImGui::Separator();

  ImGui::End();
}

static void menu_gpu_profiler() {
  cpu_profile p("menu_gpu_profiler");

  ImGui::Begin("GPU profiler");

  ImGui::Text("Frametime: %.3f", smoothed_render_frametime);

  ImGui::Columns(3, "mycolumns");
  ImGui::Separator();
  ImGui::Text("Function(s)");
  ImGui::NextColumn();
  ImGui::Text("Time (ms)");
  ImGui::NextColumn();
  ImGui::Text("%% of total");
  ImGui::NextColumn();

  ImGui::Separator();

  const auto col = ImVec4(0.5f, 1.0f, 0.8f, 1.0f);

  // if you want it sorted by time taken
  // auto new_map = flip_map(time_taken_by);

  for (const auto &[id, time] : gpu_profiles) {
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::Text("%s", id.c_str());
    ImGui::NextColumn();
    ImGui::PopStyleColor();

    ImGui::Text("%f", time);
    ImGui::NextColumn();

    auto perc = 100.0 * time / frametime;
    // If % is over 50% color red
    if (perc > 50) {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.5f, 0.5f, 1.0f));
      ImGui::Text("%.3f", perc);
      ImGui::PopStyleColor();
    } else {
      ImGui::Text("%.3f", perc);
    }
    ImGui::NextColumn();
  }
  gpu_profiles.clear();

  ImGui::Columns(1);
  ImGui::Separator();

  ImGui::End();
}

static void renderer_submenu()
{
    ImGui::Checkbox("VSync", &vsync);
    ImGui::SameLine();
    ImGui::InputInt("framerate limit", &framerate_limit, 0, 1000);
    ImGui::Checkbox("draw nodes ", &draw_nodes);
    ImGui::SameLine();
    ImGui::Checkbox("color particles based on node", &color_particles);
}

static void simulation_submenu()
{
    ImGui::InputInt("Physics samples", &physics_samples);
    if (physics_samples < 1) physics_samples = 1;
    ImGui::Checkbox("Multithreaded particle update", &multithreaded_particle_update);
    ImGui::Checkbox("Particle intercollision", &circle_collision);
    ImGui::SameLine();
    ImGui::Checkbox("Border Collisions", &border_collision);
    ImGui::SliderFloat("time scale", &time_scale, 0.0001f, 2.0f);
    ImGui::SliderFloat(" ", &gravity, 0.01f, 20.0f);

    ImGui::Checkbox("gravitational force", &use_gravitational_force);
}

static int vertices_to_be_applied = 36;
static void menu_settings() {
  cpu_profile p("menu_settings");

  ImGui::Begin("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::PushItemWidth(150.0f);

  if (ImGui::CollapsingHeader("Renderer")) {
    renderer_submenu();
  }

  if (ImGui::CollapsingHeader("Post-processing")) {
  ImGui::Checkbox("Post-processing on", &post_processing);
  ImGui::InputInt("Post-processing samples", &post_processing_samples);
  ImGui::InputInt("Blur strength", &blur_strength);
  if (physics_samples < 1)
    physics_samples = 1;

  if (post_processing_samples < 1)
    post_processing_samples = 1;

  if (blur_strength < 1)
    blur_strength = 1;
  }

  if (ImGui::CollapsingHeader("Simulation")) {
    simulation_submenu();
  }

        // Multithreading options
  if (ImGui::CollapsingHeader("multithreading options")) {
    // Only setup for Apple systems. Linux in the future.
#ifdef __APPLE__
    ImGui::Checkbox("libdispatch", &use_libdispatch);
    ImGui::SameLine();
#endif
    ImGui::Checkbox("OpenCL", &openCL_active);

    ImGui::Checkbox("multithreaded", &use_multithreading);
    ImGui::SameLine();
    ImGui::InputInt("", &variable_thread_count);
    if (variable_thread_count < 0)
      variable_thread_count = 0;
  }

  if (ImGui::CollapsingHeader("quadtree options")) {
    ImGui::Checkbox("Occupied only", &quadtree_show_only_occupied);
    ImGui::SliderInt("depth", &quadtree_depth, 0, 10);
    ImGui::SliderInt("capacity", &quadtree_capacity, 0, 100);
  }
  if (ImGui::CollapsingHeader("uniform grid options")) {
    ImGui::SliderInt("nodes", &uniformgrid_parts, 4, 1024);
  }

  if (ImGui::CollapsingHeader("particle options")) {

    // Initial state

    ImGui::Checkbox("give random velocity", &has_random_velocity);
    if (has_random_velocity)
      ImGui::SliderFloat("random starting force", &random_velocity_force, 0.1f, 10.0f);


    // Color changed by acceleration
    ImGui::Checkbox("colored by acceleration", &is_particles_colored_by_acc);
    if (is_particles_colored_by_acc) {
        ImGui::SliderFloat("velocity threshold", &color_by_velocity_threshold, 0.001f, 0.05f);
        ImGui::Text("minimum");
        ImGui::SameLine(200);
        ImGui::Text("maximum");
        ImGui::ColorPicker4("##particle_acc_min_color", (float *)&acceleration_color_min);
        ImGui::SameLine();
        ImGui::ColorPicker4("##particle_acc_max_color", (float *)&acceleration_color_max);
    }

    ImGui::Separator();

    // Rebuild particle vertices
    ImGui::InputInt("Vertices per particle", &vertices_to_be_applied, 1, 999);
    if (vertices_to_be_applied != num_vertices_per_particle) {
      if (vertices_to_be_applied < 3) vertices_to_be_applied = 3;
      particle_system.rebuild_vertices(vertices_to_be_applied);
    }

    ImGui::Separator();

    // Change all particles color
    ImGui::Text("particle color");
    ImGui::SameLine();
    if (ImGui::SmallButton("Color: Apply to all")) {
      for (auto &p : particle_system.particles)
        particle_system.colors[p.id] = circle_color;
    }
    ImGui::ColorPicker4("##particle", (float *)&circle_color);

    // Particle energy loss
    ImGui::Text("collision energy loss");
    ImGui::SameLine();
    ImGui::InputFloat("energy loss", &collision_energy_loss, 0.01f, 1.0f);

    // Change all particles radius
    ImGui::Text("particle radius");
    ImGui::SliderFloat("radius", &circle_size, 0.1f, 10.0f);
    ImGui::SameLine();
    if (ImGui::SmallButton("Radius: Apply to all")) {
      for (auto &p : particle_system.particles) {
        p.radius = circle_size;
        particle_system.transforms[p.id].scale = glm::vec3(circle_size, circle_size, 0);
      }
    }
  }
  if (ImGui::CollapsingHeader("color options")) {
    ImGui::Text("background color");
    ImGui::ColorPicker4("##Background", (float *)&background_color);
  }
  ImGui::PopItemWidth();
  ImGui::End();
}

void gui_render() {
  cpu_profile p("gui_render");

  ImGui_ImplGlfwGL3_NewFrame();

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("menu")) {
      ImGui::MenuItem("settings", NULL, &open_settings);
      if constexpr (DEBUG_MODE) {

        ImGui::MenuItem("GPU profiler", NULL, &open_gpu_profiler);
        ImGui::MenuItem("CPU profiler", NULL, &open_cpu_profiler);
      }
      if constexpr (DEBUG_MODE) ImGui::MenuItem("debug", NULL, &open_debug_menu);
      ImGui::EndMenu();
    }

    ImGui::RadioButton("Quadtree", &tree_radio_option, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Uniform Grid", &tree_radio_option, 1);
    ImGui::SameLine();
    ImGui::RadioButton("None", &tree_radio_option, 2);

    switch ((TreeType)tree_radio_option) {
      case TreeType::Quadtree: {
        quadtree_active = true;
        use_uniformgrid = false;
        tree_type = TreeType::Quadtree;
      } break;
      case TreeType::UniformGrid: {
        use_uniformgrid = true;
        quadtree_active = false;
        tree_type = TreeType::UniformGrid;
      } break;
      case TreeType::None: {
        quadtree_active = false;
        use_uniformgrid = false;
        tree_type = TreeType::None;
      } break;
    }

    ImGui::RadioButton("Color", &mouse_radio_options, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Gravity Well", &mouse_radio_options, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Drag", &mouse_radio_options, 2);
    ImGui::SameLine();
    ImGui::RadioButton("Delete", &mouse_radio_options, 3);
    ImGui::SameLine();
    ImGui::RadioButton("None", &mouse_radio_options, 4);

    switch ((MouseOption)mouse_radio_options) {
      case MouseOption::Color: { mouse_option = MouseOption::Color; } break;
      case MouseOption::GravityWell: { mouse_option = MouseOption::GravityWell; } break;
      case MouseOption::Drag: { mouse_option = MouseOption::Drag; } break;
      case MouseOption::Delete: { mouse_option = MouseOption::Delete; } break;
      case MouseOption::None: { /* DO NOTHING */ } break;
    }

    const auto red = ImVec4(1.0f, 0.1f, 0.1f, 1.0f);
    const auto green = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, (framerate >= monitor_refreshrate) ? green : red);
    ImGui::Text("FPS %d", framerate);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, (physics_framerate >= monitor_refreshrate) ? green : red);
    ImGui::Text("Physics FPS %d", physics_framerate);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    const auto yellow = ImVec4(0.1f, 8.0f, 0.8f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, yellow);
    ImGui::Text("particles: %lu", particle_system.particles.size());
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushItemWidth(100.0f);
    ImGui::SliderFloat("particle size", &circle_size, 1.0f, 100.0f);
    ImGui::SameLine();
    ImGui::SliderFloat("mouse size", &mouse_size, 1.0f, 500.0f);
    ImGui::SameLine();
    ImGui::PopItemWidth();
    ImGui::Checkbox("grab", &mouse_grab);
    ImGui::SameLine();

    ImGui::EndMainMenuBar();
  }

  if (open_settings)
    menu_settings();
  if constexpr (DEBUG_MODE) {
    if (open_cpu_profiler) menu_cpu_profiler();
    if (open_gpu_profiler) menu_gpu_profiler();
    if (open_debug_menu) menu_debug();
  }

  ImGui::Render();
}

void gui_init(GLFWwindow *window, float px_scale) {
  ImGui_ImplGlfwGL3_Init(window, false);
  ImGuiIO &io = ImGui::GetIO();

  io.FontGlobalScale = 1.0f / px_scale;

  #ifdef _WIN32
  px_scale = 2;
  #endif
  io.Fonts->AddFontFromFileTTF("../Resources/Fonts/DroidSans.ttf", 14 * px_scale, NULL, io.Fonts->GetGlyphRangesJapanese());
  // SetupImGuiStyle(true, 1.0f);
  new_style();
}

void gui_shutdown() {
  // font_renderer::shutdown();
 ImGui_ImplGlfwGL3_Shutdown();
}

static void new_style() {
  ImGuiStyle *style = &ImGui::GetStyle();

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
  style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
  style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
  style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
  style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.09f, 0.12f, 1.0f);
  style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
  style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  //style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
  style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
  style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
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
  style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
  style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
  style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
  style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
  style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
  style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
  style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
  style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
  style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 1.00f);
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
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
  style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
  style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
  style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrab] = imgui_pastel_red;
  style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
  //style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_SliderGrab] = imgui_pastel_red;
  style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Header] = imgui_pastel_red;
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ResizeGrip] = imgui_pastel_red;
  style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
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
