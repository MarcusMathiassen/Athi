#pragma once

#include "athi_typedefs.h"

class Athi_Core_Renderer
{
private:
public:
  s32 framerate_limit{30}; //
  u32 framerate {0};     // Rendering framerate(frames/sec)
  f64 frametime {0.0};   // Rendering frametime(ms/frame)
  bool vsync {false};

  void UI();
  void init();
  void update();
};
