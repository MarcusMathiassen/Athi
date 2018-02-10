// Copyright (c) 2018 Marcus Mathiassen

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#pragma once

#include "../athi_typedefs.h"

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, DOWN, UP };

// Default camera values
const f32 YAW = -90.0f;
const f32 PITCH = 0.0f;
const f32 SPEED = 4.5f;
const f32 SENSITIVTY = 0.1f;
const f32 ZOOM = 45.0f;

struct Camera {
  // Camera Attributes
  vec3 position{0.0f, 0.0f, 0.0f};
  vec3 front{0.0f, 0.0f, -1.0f};
  vec3 up{0, 1, 0};
  vec3 right;
  vec3 world_up{0, 1, 0};

  // Eular Angles
  f32 yaw{YAW};
  f32 pitch{PITCH};

  // Camera options
  f32 movement_speed{SPEED};
  f32 mouse_sensitivity{SENSITIVTY};
  f32 zoom{ZOOM};

  f32 fov{45.0f};
  f32 aspect_ratio{1280 / 720};
  f32 zNear{0.1f};
  f32 zFar{1000.0f};
  f32 moveSpeed{0.02f};

  mat4 view_matrix;
  mat4 *active_projection;
  mat4 perspective_projection;
  mat4 ortho_projection;

  Camera() { update_camera_vectors(); }

  void update();
  void update_projection(f32 width, f32 height);
  void update_ortho_projection(f32 left, f32 bottom, f32 width, f32 height);

  mat4 get_view_matrix() const;
  mat4 get_view_projection() const;
  mat4 get_perspective_projection() const;
  mat4 get_ortho_projection() const;

  void process_mouse_scroll(f32 yoffset);
  void process_mouse_movement(f32 xoffset, f32 yoffset,
                              bool constrain_pitch = true);
  void process_keyboard(Camera_Movement direction, f32 deltaTime);
  void update_camera_vectors();
  void use_projection_ortho();
  void use_projection_perspective();
};

extern Camera camera;
