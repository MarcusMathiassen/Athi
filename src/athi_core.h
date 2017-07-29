#pragma once

#include "athi_typedefs.h"
#include "athi_window.h"
#include "athi_settings.h"
#include "athi_ui.h"
#include "athi_text.h"

#include <memory>

class Athi_Core
{
private:

  f32 frametime;
  u32 framerate;
  s32 framerate_limit;
  bool vsync;

  u32    cpu_cores;
  u32    cpu_threads;
  string cpu_brand;

  bool app_is_running{true};
  bool settings_changed{false};

  std::unique_ptr<Athi_UI_Manager>      ui_manager;
  std::unique_ptr<Athi_Window>          window;
  std::unique_ptr<Athi_Text_Manager>     text_manager;

public:

  void draw_UI();
  void draw_loop();

  void init();
  void shutdown();

  void start();

  void update_settings();

  template <typename T>
  void add_text_dynamic(T* str, f32 x, f32 y, std::string id);
};
