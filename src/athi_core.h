#pragma once

#include "athi_typedefs.h"
#include "athi_window.h"
#include "athi_settings.h"
#include "athi_ui.h"
#include "athi_slider.h"

#include <memory>

struct Athi_Core
{
  u32 framerate;
  s32 framerate_limit{60};

  f64 frametime;
  f64 smoothed_frametime;

  bool vsync{1};

  u32    cpu_cores;
  u32    cpu_threads;
  string cpu_brand;

  bool app_is_running{true};
  bool settings_changed{false};

  std::unique_ptr<Athi_UI_Manager>      ui_manager;
  std::unique_ptr<Athi_Window>          window;

  void update_UI();
  void draw_UI();
  void draw_loop();

  void init();
  void shutdown();

  void start();

  void update_settings();
};
