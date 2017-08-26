#pragma once

#include <memory>

#include "athi_physics_engine.h"
#include "athi_settings.h"
#include "athi_slider.h"
#include "athi_task_manager.h"
#include "athi_typedefs.h"
#include "athi_ui.h"
#include "athi_window.h"

struct Athi_Core {
  std::unique_ptr<Athi_Window> window;
  std::unique_ptr<Athi_Window> debug_window;
  PhysicsEngine physics_engine;
  TaskManager thread_manager;

  void window_loop();
  void draw_loop();
  void physics_loop();

  void init();
  void shutdown();

  void start();

  void update_settings();
};
