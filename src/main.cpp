
#include "athi.h"

int main()
{
  Athi_Core athi;
  athi.init();

  athi.framerate_limit = 60;
  athi.vsync = 1;

  auto slider = create_slider<s32>(&athi.framerate_limit);
  slider->pos = vec2(LEFT+ROW*0.3f, BOTTOM+ROW*2.5f);
  slider->width = 0.5f;
  slider->height = 0.03f;
  slider->knob_width = 0.03f;
  slider->min = 0;
  slider->max = 300;
  slider->init();
  //add_UI(&slider);
  athi.ui_manager->ui_buffer.emplace_back(std::move(slider));

  //auto vsync_checkbox = create_checkbox();
  //vsync_checkbox.pos = vec2(LEFT BOTTOM+ROW*6);

  athi.start();
}
