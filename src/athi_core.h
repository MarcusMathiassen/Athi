#pragma once

#include "athi_typedefs.h"
#include "athi_core_renderer.h"
#include "athi_window.h"
#include "athi_settings.h"

#include <memory>

class Athi_Core
{
private:
  u32    cpu_cores;
  u32    cpu_threads;
  string cpu_brand;

  bool app_is_running{true};
  std::unique_ptr<Athi_Core_Renderer>   renderer; // Only place GPU calls are made
  std::unique_ptr<Athi_Window>          window;   // Window and Input handler

public:
  void UI();
  void draw_loop();

  void init();
  void shutdown();

  void start();
};
