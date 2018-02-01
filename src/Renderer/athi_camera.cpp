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

#include "athi_camera.h"

#include <glm/gtc/matrix_transform.hpp>  // glm::radians, glm::normalize, glm::cross, glm::perspective, glm::lookAt

Camera camera;

mat4 Camera::get_view_projection() const {
  return *active_projection * view_matrix;
}
mat4 Camera::get_view_matrix() const { return view_matrix; }

void Camera::process_mouse_movement(f32 xoffset, f32 yoffset,
                                    bool constrain_pitch) {
  xoffset *= mouse_sensitivity;
  yoffset *= mouse_sensitivity;

  yaw += xoffset;
  pitch += yoffset;

  // Make sure that when pitch is out of bounds, screen doesn't get flipped
  if (constrain_pitch) {
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
  }

  // Update front, right and Up Vectors using the updated Eular angles
  update_camera_vectors();
}

void Camera::update_camera_vectors() {
  // Calculate the new front vector
  glm::vec3 n_front;
  n_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  n_front.y = sin(glm::radians(pitch));
  n_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  front = glm::normalize(n_front);
  // Also re-calculate the right and Up vector
  right =
      glm::normalize(glm::cross(front, world_up));  // Normalize the vectors,
                                                    // because their length
                                                    // gets closer to 0 the
                                                    // more you look up or
                                                    // down which results in
                                                    // slower movement.
  up = glm::normalize(glm::cross(right, front));
}

void Camera::process_mouse_scroll(f32 yoffset) {
  if (zoom >= 44.0f && zoom <= 46.0f) zoom -= yoffset;
  if (zoom <= 44.0f) zoom = 44.0f;
  if (zoom >= 46.0f) zoom = 46.0f;
}

void Camera::process_keyboard(Camera_Movement direction, f32 deltaTime) {
  const f32 velocity = movement_speed * deltaTime;
  if (direction == FORWARD) position += front * velocity;
  if (direction == BACKWARD) position -= front * velocity;
  if (direction == LEFT) position -= right * velocity;
  if (direction == RIGHT) position += right * velocity;
  if (direction == DOWN) position -= up * velocity;
  if (direction == UP) position += up * velocity;
}

void Camera::update() {
  view_matrix = glm::lookAt(position, position + front, up);
  perspective_projection = glm::perspective(zoom, aspect_ratio, zNear, zFar);
}

void Camera::update_projection(f32 width, f32 height) {
  aspect_ratio = width / height;
  perspective_projection = glm::perspective(zoom, aspect_ratio, zNear, zFar);

  ortho_projection = glm::ortho(0.0f, width, 0.0f, height);
}

mat4 Camera::get_perspective_projection() const {
  return perspective_projection;
}

mat4 Camera::get_ortho_projection() const { return ortho_projection; }

void Camera::use_projection_ortho() { active_projection = &ortho_projection; }

void Camera::use_projection_perspective() {
  active_projection = &perspective_projection;
}
