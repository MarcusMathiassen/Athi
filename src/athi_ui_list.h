#pragma once

#include "athi_typedefs.h"
#include "athi_text.h"

struct Athi_UI_List
{
  vec2 pos{0.0f,0.0f};
  int num_rows{0};
  vector<Athi_Text> text_buffer;

  Athi_UI_List() = default;

  void add(Athi_Text &text)
  {
    text.pos = vec2(pos.x, pos.y-ROW*num_rows++);
    text_buffer.emplace_back(text);
  }
  void clear()
  {
    text_buffer.clear();
  }
  void draw()
  {
    for (const auto& text: text_buffer)
    {
      draw_text(text.str, text.pos, text.color);
    }
  }
};
