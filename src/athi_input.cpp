#include "athi_input.h"

void init_input_manager() { athi_input_manager.init(); }
u8 get_mouse_button_state(u8 button)
{
  switch(button)
  {
    case GLFW_MOUSE_BUTTON_LEFT:  return athi_input_manager.mouse.left_button.state;
    case GLFW_MOUSE_BUTTON_RIGHT: return athi_input_manager.mouse.right_button.state;
  }
  return 2;
}
