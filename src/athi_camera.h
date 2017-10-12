#pragma once

#include "athi_typedefs.h"

#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct Camera {
  vec3 position{0.0f, 0.0f, 3.0f};
  vec3 forward{0.0f, 0.0f, -1.0f};
  vec3 up{0.0f, 1.0f, 0.0f};

  f32 fov{75.0f};
  f32 aspect_ratio{1.0f};
  f32 zNear{0.01f};
  f32 zFar{1000.0f};
  f32 zoom{1.0f};
  f32 window_width{512.0f}, window_height{512.0f};
  mat4 perspective{glm::perspective(fov, aspect_ratio, zNear, zFar)};

  f32 moveSpeed{0.02f};
  Camera() = default;
  mat4 get_view_matrix() const {
    return glm::lookAt(position, position + forward, up);
  }
  mat4 get_view_projection() const {
    return perspective * glm::lookAt(position, position + forward, up);
  }
  void update_perspective() {
    perspective =
        glm::perspective(fov * (f32)M_PI / 360.0f, aspect_ratio, zNear, zFar);
  }
};

extern Camera camera;
