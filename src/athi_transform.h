#pragma once

#include <glm/gtx/transform.hpp>
#include "athi_typedefs.h"

struct Transform {
  vec3 pos{0, 0, 0};
  vec3 rot{0, 0, 0};
  vec3 scale{1, 1, 1};

  mat4 get_model() const {
    const mat4 posMatrix{glm::translate(pos)};
    const mat4 rotXMatrix{glm::rotate(rot.x, vec3{1, 0, 0})};
    const mat4 rotYMatrix{glm::rotate(rot.y, vec3{0, 1, 0})};
    const mat4 rotZMatrix{glm::rotate(rot.z, vec3{0, 0, 1})};
    const mat4 scaleMatrix{glm::scale(scale)};
    const mat4 rotMatrix{rotZMatrix * rotYMatrix * rotXMatrix};
    return posMatrix * rotMatrix * scaleMatrix;
  }

  Transform() = default;
};
