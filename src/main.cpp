
#include "athi.h"

int main()
{
  Athi_Core athi;
  athi.init();

  athi.framerate_limit = 30;
  athi.vsync = 0;

  auto slider = create_slider<s32>(&athi.framerate_limit);
  slider->pos = vec2(LEFT, BOTTOM+ROW*3);
  slider->width = 0.8f;
  slider->height = 0.05f;
  slider->knob_width = 0.05f;
  slider->min = 30;
  slider->max = 144;
  slider->init();
  athi.ui_manager->ui_buffer.emplace_back(std::move(slider));

  athi.start();
}
