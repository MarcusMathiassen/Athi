#pragma once

#include "athi_typedefs.h"
#include "athi_window.h"
#include "athi_settings.h"
#include "athi_ui.h"
#include "athi_slider.h"

#include <memory>

struct Athi_Core
{
  std::unique_ptr<Athi_Window>          window;

  void draw_loop();

  void init();
  void shutdown();

  void start();

  void update_settings();
};
