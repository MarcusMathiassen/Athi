#pragma once

#include "athi_typedefs.h"
#include "athi_text.h"

struct Athi_UI_List
{
  vec2 pos{0.0f,0.0f}
  int num_rows{0};
  vector<Athi_Text> text_buffer;

  void add(const Athi_Text& text){
    text.pos = vec2( pos.x, pos.y-ROW*num_rows);
    buffer.emplace_back(text);
  }
  void clear()
  {
    buffer.clear();
  }
};
