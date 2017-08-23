#include "athi_input.h"
#include "athi_rect.h"
#include "athi_circle.h"
#include "athi_line.h"
#include "athi_spring.h"
#include "athi_renderer.h"

Athi_Input_Manager athi_input_manager;

void init_input_manager() { athi_input_manager.init(); }

vec2 get_mouse_viewspace_pos()
{
    auto context = glfwGetCurrentContext();  
    double mouse_pos_x, mouse_pos_y;
    glfwGetCursorPos(context, &mouse_pos_x, &mouse_pos_y);
  
    s32 width, height;
    glfwGetWindowSize(context, &width, &height);
    mouse_pos_x = -1.0f + 2 * mouse_pos_x / width;
    mouse_pos_y =  1.0f - 2 * mouse_pos_y / height;

    return vec2(mouse_pos_x, mouse_pos_y);
}

u8 get_mouse_button_state(u8 button) {
  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:
    return athi_input_manager.mouse.left_button.state;
  case GLFW_MOUSE_BUTTON_RIGHT:
    return athi_input_manager.mouse.right_button.state;
  }
  return 2;
}

// Adds force to the object 'a'
void gravitational_force(Athi_Circle &a, const vec2 &point)
{
  // Set up variables
  const f32 x1 = a.pos.x;
  const f32 y1 = a.pos.y;
  const f32 x2 = point.x;
  const f32 y2 = point.y;
  const f32 m1 = a.mass;
  const f32 m2 = 10.0f;

  // Get distance between balls.
  const f32 dx = x2 - x1;
  const f32 dy = y2 - y1;
  const f32 d  = sqrt(dx * dx + dy * dy);

  if (d != 0.0f)
  {
    const f32 angle = atan2(dy, dx);
    const f32 G = 6.674f;
    const f32 F = G * m1 * m2 / d * d;

    a.vel.x += F * cos(angle);
    a.vel.y += F * sin(angle);
  }
}

void attraction_force(Athi_Circle &a, const vec2 &point)
{
  // Set up variables
  const f32 x1 = a.pos.x;
  const f32 y1 = a.pos.y;
  const f32 x2 = point.x;
  const f32 y2 = point.y;

  // Get distance between balls.
  const f32 dx = x2 - x1;
  const f32 dy = y2 - y1;
  const f32 d  = sqrt(dx * dx + dy * dy);

  const f32 angle = atan2(dy, dx);
  a.vel.x += 0.1f*d * cos(angle);
  a.vel.y += 0.1f*d * sin(angle);
  a.vel *= 0.7f;
}


vector<u32> mouse_attached_to;
s32 mouse_attached_to_single{-1};
enum { ATTACHED, PRESSED, NOTHING };
u16 last_state{NOTHING};


s32 id1, id2;
bool found{false};
bool attach{false};
void mouse_attach_spring()
{
  // Drag a spring from one circle to the other
  // Get mouse position
  vec2 mouse_pos = get_mouse_viewspace_pos();

  Rect mouse_rect(vec2(mouse_pos.x-mouse_size, mouse_pos.y-mouse_size), vec2(mouse_pos.x+mouse_size, mouse_pos.y+mouse_size));

  s32 state = get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT);

  if (attach)
  {
    attach_spring(*athi_circle_manager->circle_buffer[id1], *athi_circle_manager->circle_buffer[id2]);
    attach = false;
  }

  // If mouse released and first circle is found
  if (state == GLFW_RELEASE && found)
  {
    for (auto& c: athi_circle_manager->circle_buffer)
    {
      if (mouse_rect.contains(c->id))
      {
        id2 = c->id;
        attach = true;
        found = false;
        return;
      }
    }
  }

  // draw a line while we look for the second circle
  if (state == GLFW_PRESS && found)
  {
      draw_line(mouse_pos, athi_circle_manager->circle_buffer[id1]->pos, 0.03f, pastel_pink);
      return;
  }

  // When the mouse is pressed down, get the first circle id
  if (state == GLFW_PRESS)
  {
    for (auto& c: athi_circle_manager->circle_buffer)
    {
      if (mouse_rect.contains(c->id))
      {
        id1 = c->id;
        found = true;
        return;
      }
    }
  }
}

void mouse_grab() {

  // Get the mouse state
  s32 state = get_mouse_button_state(GLFW_MOUSE_BUTTON_LEFT);

  auto mouse_pos = get_mouse_viewspace_pos();

  Rect mouse_rect(mouse_pos - mouse_size, mouse_pos + mouse_size);
  //draw_hollow_rect(mouse_rect.min, mouse_rect.max, pastel_green);

  // If it's released just exit the function
  if (state == GLFW_RELEASE)
  {
    last_state = NOTHING;
    mouse_busy_UI = false;

    // remove all circles attached
    mouse_attached_to.clear();
    return;
  }
  // If pressed continue on..

  // If already attached
  if (last_state == ATTACHED)
  {
    if (mouse_grab_multiple)
    {
      for (auto& id: mouse_attached_to)
      {
        attraction_force(*athi_circle_manager->circle_buffer[id], mouse_pos);
        last_state = ATTACHED;
        draw_line(mouse_pos, athi_circle_manager->circle_buffer[id]->pos, 0.03f, pastel_green);
      }
    }
    else // single
    {
      attraction_force(*athi_circle_manager->circle_buffer[mouse_attached_to_single], mouse_pos);
      draw_line(mouse_pos, athi_circle_manager->circle_buffer[mouse_attached_to_single]->pos, 0.03f, pastel_green);
    }
    mouse_busy_UI = true;
  }


  // Go through all circles. Return the circle hovered
  if (last_state != ATTACHED)
  for (auto& c: athi_circle_manager->circle_buffer)
  {
    // If the mouse and circle intersect
    if (mouse_rect.contains(c->id))
    {
      if (mouse_grab_multiple)
      {
        mouse_attached_to.emplace_back(c->id);
      }
      else
      {
        mouse_attached_to_single = c->id;
      }
      last_state = ATTACHED;
    }
  }
}

void update_inputs() {

  auto mouse_pos = get_mouse_viewspace_pos();
  auto context = glfwGetCurrentContext();

  // Find the circle you're over
  if (glfwGetKey(context, GLFW_KEY_S) == GLFW_PRESS) {
    mouse_attach_spring();
  } else mouse_grab();


  Athi_Circle c;
  if (glfwGetKey(context, GLFW_KEY_SPACE) == GLFW_PRESS) {
    c.pos = mouse_pos;
    c.radius = circle_size;
    add_circle(c);
  }
  if (glfwGetKey(context, GLFW_KEY_1) == GLFW_PRESS) {
    c.pos = mouse_pos;
    c.radius = 0.003f;
    for (int i = 0; i < 10; ++i)
      add_circle(c);
  }
  if (glfwGetKey(context, GLFW_KEY_2) == GLFW_PRESS) {
    c.pos = mouse_pos;
    c.radius = 0.005f;
    add_circle(c);
  }
  if (glfwGetKey(context, GLFW_KEY_3) == GLFW_PRESS) {
    c.pos = mouse_pos;
    c.radius = 0.007f;
    add_circle(c);
  }

  //  CAMERA MOVE
  if (glfwGetKey(context, GLFW_KEY_LEFT) == GLFW_PRESS) {
    camera.position.x -= 1;
  }
  //  CAMERA MOVE
  if (glfwGetKey(context, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    camera.position.x += 1;
  }
  //  CAMERA MOVE
  if (glfwGetKey(context, GLFW_KEY_DOWN) == GLFW_PRESS) {
    camera.position.y -= 1;
  }
  //  CAMERA MOVE
  if (glfwGetKey(context, GLFW_KEY_UP) == GLFW_PRESS) {
    camera.position.y += 1;
  }

  if (glfwGetKey(context, GLFW_KEY_A) == GLFW_PRESS) {
    camera.zoom -= 0.01f;
  }
  //  CAMERA MOVE
  if (glfwGetKey(context, GLFW_KEY_S) == GLFW_PRESS) {
    camera.zoom += 0.01f;
  }
  camera.update_perspective();
}
