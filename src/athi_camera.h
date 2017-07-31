#pragma once

#include "athi_typedefs.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct Camera
{
  vec3 position{0, 0, -5};
  vec3 forward{0, 0, 1};
  vec3 up{0, 1, 0};

  f32 fov{120.0f};
  f32 aspect_ratio{1.0f};
  f32 zNear{0.01f};
  f32 zFar{1000.0f};
  mat4 perspective{glm::perspective(fov, aspect_ratio, zNear, zFar)};

  f32 moveSpeed{0.02f};
  Camera() = default;
  mat4 get_view_projection() const
  {
    return perspective * glm::lookAt(position, position + forward, up);
  }
  void update_perspective()
  {
    perspective = glm::perspective(fov, aspect_ratio, zNear, zFar);
  }
};

extern Camera camera;
