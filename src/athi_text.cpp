#include "athi_text.h"

void init_text_manager()
{
  athi_text_manager = new Athi_Text_Manager;
  athi_text_manager->init();
}

void shutdown_text_manager()
{
  delete athi_text_manager;
}

void add_text(Athi_Text *text)
{
  text->init();
  athi_text_manager->text_buffer.emplace_back(text);
}

void draw_all_text()
{
  athi_text_manager->draw();
}
