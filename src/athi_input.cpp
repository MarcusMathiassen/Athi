#include "athi_input.h"

Athi_Input_Manager athi_input_manager;
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

void update_inputs()
{
  f64 mouse_x, mouse_y;
  glfwGetCursorPos(glfwGetCurrentContext(), &mouse_x, &mouse_y);

  s32 width, height;
  glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
  mouse_x = -1.0f + 2 * mouse_x / width;
  mouse_y = +1.0f - 2 * mouse_y / height;

  Athi_Circle c;
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_SPACE) == GLFW_PRESS)
  {
    c.pos = vec2(mouse_x,mouse_y);
    c.radius = circle_size;
    add_circle(c);
  }
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_1) == GLFW_PRESS)
  {
    c.pos = vec2(mouse_x,mouse_y);
    c.radius = 0.003f;
    add_circle(c);
  }
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_2) == GLFW_PRESS)
  {
    c.pos = vec2(mouse_x,mouse_y);
    c.radius = 0.005f;
    add_circle(c);
  }
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_3) == GLFW_PRESS)
  {
    c.pos = vec2(mouse_x,mouse_y);
    c.radius = 0.007f;
    add_circle(c);
  }


    //  CAMERA MOVE
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_LEFT) == GLFW_PRESS)
  {
    camera.position.x -= 1;
  }
    //  CAMERA MOVE
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_RIGHT) == GLFW_PRESS)
  {
    camera.position.x += 1;
  }
    //  CAMERA MOVE
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_DOWN) == GLFW_PRESS)
  {
    camera.position.y -= 1;
  }
    //  CAMERA MOVE
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_UP) == GLFW_PRESS)
  {
    camera.position.y += 1;
  }

    if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_A) == GLFW_PRESS)
  {
    camera.zoom -= 0.01f;
  }
    //  CAMERA MOVE
  if (glfwGetKey(glfwGetCurrentContext(),GLFW_KEY_S) == GLFW_PRESS)
  {
    camera.zoom += 0.01f;
  }
  camera.update_perspective();
}
