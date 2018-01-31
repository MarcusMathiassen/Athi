#pragma once

#include "athi_typedefs.h"

#include "athi_particle.h" // particle_system
#include "athi_settings.h" // has_random_velocity, etc.

#include "../dep/Universal/imgui.h"
#include "../dep/Universal/imgui_impl_glfw_gl3.h"

static bool open_settings = false;
static bool open_profiler = false;
static bool open_debug_menu = false;

void gui_init(GLFWwindow *window, float px_scale);
void gui_shutdown();
void gui_render();

static void new_style();
static void SetupImGuiStyle(bool bStyleDark_, float alpha_);
static void menu_profiler();
static void menu_settings();

template <typename A, typename B> static std::pair<B, A> flip_pair(const std::pair<A, B> &p) {
  return std::pair<B, A>(p.second, p.first);
}

// flips an associative container of A,B pairs to B,A pairs
template <typename A, typename B, template <class, class, class...> class M, class... Args>
static std::multimap<B, A> flip_map(const M<A, B, Args...> &src) {
  std::multimap<B, A> dst;
  std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()), flip_pair<A, B>);
  return dst;
}

static void menu_debug() {
  ImGui::Begin("Debug Options", NULL, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::Checkbox("give random velocity", &has_random_velocity);
  ImGui::Checkbox("Particles colored by acceleration", &is_particles_colored_by_acc);
  ImGui::Checkbox("mouse_grab", &mouse_grab);
  ImGui::Checkbox("show_mouse_collision_box", &show_mouse_collision_box);
  ImGui::Checkbox("show_mouse_grab_lines", &show_mouse_grab_lines);
  ImGui::Checkbox("draw_debug", &draw_debug);
  ImGui::Checkbox("color_particles", &color_particles);
  ImGui::Checkbox("draw_nodes", &draw_nodes);
  ImGui::Checkbox("quadtree_show_only_occupied", &quadtree_show_only_occupied);
  ImGui::Checkbox("quadtree_active", &quadtree_active);
  ImGui::Checkbox("use_uniformgrid", &use_uniformgrid);
  ImGui::Checkbox("vsync", &vsync);
  ImGui::Checkbox("use_gravitational_force", &use_gravitational_force);
  ImGui::Checkbox("physics_gravity", &physics_gravity);
  ImGui::Checkbox("use_multithreading", &use_multithreading);
  ImGui::Checkbox("use_libdispatch", &use_libdispatch);
  ImGui::Checkbox("openCL_active", &openCL_active);
  ImGui::Checkbox("post_processing", &post_processing);

  ImGui::End();
}

static void menu_profiler() {
  profile p("menu_profiler");

  ImGui::Begin("Profiler");

  ImGui::Text("Frametime: %.3f", frametime);

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

  for (const auto &[id, time] : time_taken_by) {
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
  time_taken_by.clear();

  ImGui::Columns(1);
  ImGui::Separator();

  ImGui::End();
}


static int vertices_to_be_applied = 36;

static void menu_settings() {
  profile p("menu_settings");

  ImGui::Begin("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::InputInt("Physics samples", &physics_samples);

  ImGui::Checkbox("VSync", &vsync);
  ImGui::SameLine();
  ImGui::Checkbox("Particle Collisions", &circle_collision);
  ImGui::SameLine();
  ImGui::Checkbox("Border Collisions", &border_collision);
  ImGui::SliderFloat("time scale", &time_scale, 0.0001f, 2.0f);

  ImGui::InputInt("framerate limit", &framerate_limit, 0, 1000);

  ImGui::Checkbox("gravitational force", &use_gravitational_force);
  ImGui::SameLine();
  ImGui::SliderFloat("", &gravitational_constant,  100, 10000);

  ImGui::Checkbox("gravity", &physics_gravity);
  ImGui::SameLine();
  ImGui::SliderFloat(" ", &gravity_force, 0.01f, 20.0f);

  ImGui::Checkbox("draw nodes ", &draw_nodes);
  ImGui::SameLine();
  ImGui::Checkbox("color particles based on node", &color_particles);

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


    ImGui::PushItemWidth(100.0f);
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
    if (ImGui::Button("Apply vertices")) {
      if (vertices_to_be_applied < 3) vertices_to_be_applied = 3;
      particle_system.rebuild_vertices(vertices_to_be_applied);
    }
    ImGui::SameLine();
    ImGui::InputInt("Vertices per particle", &vertices_to_be_applied, 3, 999);

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
    ImGui::PopItemWidth();
  }
  if (ImGui::CollapsingHeader("color options")) {
    ImGui::PushItemWidth(100.0f);
    ImGui::Text("background color");
    ImGui::ColorPicker4("##Background", (float *)&background_color_dark);

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
  profile p("gui_render");

  ImGui_ImplGlfwGL3_NewFrame();

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("menu")) {
      ImGui::MenuItem("settings", NULL, &open_settings);
      if constexpr (ONLY_RUNS_IN_DEBUG_MODE) ImGui::MenuItem("profiler", NULL, &open_profiler);
      if constexpr (ONLY_RUNS_IN_DEBUG_MODE) ImGui::MenuItem("debug", NULL, &open_debug_menu);
      ImGui::EndMenu();
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

    ImGui::Text("comparisons: %llu", static_cast<uint64_t>(comparisons) / physics_samples);
    ImGui::SameLine();
    ImGui::Text("resolved: %llu (%.4f%%)", static_cast<uint64_t>(resolutions),
                100.0f * static_cast<float>(resolutions) / static_cast<float>(comparisons));

    ImGui::EndMainMenuBar();
  }

  if (open_settings)
    menu_settings();
  if constexpr (ONLY_RUNS_IN_DEBUG_MODE) {
    if (open_profiler)
      menu_profiler();
    if (open_debug_menu)
      menu_debug();
  }

  ImGui::Render();
}

void gui_init(GLFWwindow *window, float px_scale) {
  ImGui_ImplGlfwGL3_Init(window, false);
  ImGuiIO &io = ImGui::GetIO();

  io.FontGlobalScale = 1.0f / px_scale;
  io.Fonts->AddFontFromFileTTF("../Resources/Fonts/DroidSans.ttf", 14 * px_scale, NULL, io.Fonts->GetGlyphRangesJapanese());
  // SetupImGuiStyle(true, 1.0f);
  new_style();
}

void gui_shutdown() { ImGui_ImplGlfwGL3_Shutdown(); }

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
  style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
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
  style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
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
