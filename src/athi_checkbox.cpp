#include "athi_checkbox.h"

void Athi_Checkbox::draw() const
{
  outer_box.draw();
  if (*variable || last_state == HOVER) inner_box.draw();
}

void Athi_Checkbox::update()
{
  u8 this_state = get_status();
  if (last_state == PRESSED && get_status() == HOVER) this_state = TOGGLE;
  switch(this_state)
  {
    case HOVER:       inner_box.color = hover_color;     break;
    case PRESSED:     inner_box.color = pressed_color;    break;
    case IDLE:        inner_box.color = idle_color;     break;
    case TOGGLE:      inner_box.color = pressed_color; *variable = !(*variable); break;
  }
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
    mouse_x > outer_box.pos.x && mouse_x < outer_box.pos.x+outer_box.width &&
    mouse_y > outer_box.pos.y && mouse_y < outer_box.pos.y+outer_box.height)
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
  // Border
  outer_box.pos = pos;
  outer_box.color = outer_box_color;
  outer_box.width  = width;
  outer_box.height = height;
  outer_box.init();

  // Box
  inner_box.pos = pos;
  inner_box.color = inner_box_color;
  inner_box.width  = width;
  inner_box.height = height;
  inner_box.init();

  text.pos = pos;
  text.pos.x += width;
  text.pos.y -= height*0.65f;
  add_text(&text);
}
