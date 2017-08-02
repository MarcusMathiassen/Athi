#include "athi_text.h"

std::vector<Athi_Text*> text_buffer;

void init_text_manager()
{
  athi_text_manager.init();
}

void add_text(Athi_Text* text)
{
  text->init();
  text_buffer.emplace_back(text);
}

void draw_all_text()
{
  athi_text_manager.draw();
}
