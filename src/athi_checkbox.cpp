#include "athi_checkbox.h"

void Athi_Checkbox::draw() const
{
}

void Athi_Checkbox::update()
{
  u8 this_state = get_status();
  if (last_state == PRESSED && get_status() == HOVER) this_state = TOGGLE;
  switch(this_state)
  {
    case HOVER:       box.color = hover_color;    break;
    case PRESSED:     box.color = pressed_color;  break;
    case IDLE:        box.color = box_color;     break;
    case TOGGLE:      box.color = idle_color; *variable = !(*variable); break;
  }
  if (*variable && this_state != HOVER) box.color = active_color;
  last_state = get_status();
}

bool Athi_Checkbox::hover_over()
{
  f64 mouse_x, mouse_y;
  glfwGetCursorPos(glfwGetCurrentContext(), &mouse_x, &mouse_y);

  int width, height;
  glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
  mouse_x = -1.0f + 2 * mouse_x / width;
  mouse_y = +1.0f - 2 * mouse_y / height;

  if (
    mouse_x > box.pos.x && mouse_x < box.pos.x+box.width &&
    mouse_y > box.pos.y && mouse_y < box.pos.y+box.height)
  {
    return true;
  }
  return false;
}


u32 Athi_Checkbox::get_status()
{
  if (hover_over())
  {
    if (get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) return PRESSED;
    return HOVER;
  }
  return IDLE;
}

void Athi_Checkbox::init()
{
  box.pos = pos;
  box.color = box_color;
  box.width  = width;
  box.height = height;
  add_rect(&box);

  text.pos = pos;
  text.pos.x += width+0.02f;
  text.pos.y -= height*0.65f;
  add_text(&text);
}
