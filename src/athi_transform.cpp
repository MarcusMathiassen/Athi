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

#include "athi_transform.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>    // glm::translate, glm::rotate, glm::scale
#include <glm/gtx/quaternion.hpp>   // glm::quat, glm::toMat4

glm::mat4 Transform::get_model() const noexcept
{
    const glm::mat4 posMatrix   = glm::translate(pos);
    const glm::mat4 scaleMatrix = glm::scale(scale);
    const glm::mat4 rotMatrix   = glm::toMat4(glm::quat(rot));
    return posMatrix * rotMatrix * scaleMatrix;
}

// Free functions //
glm::mat4 get_model_matrix(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) noexcept
{
  return glm::translate(position) * glm::toMat4(glm::quat(rotation)) * glm::scale(scale);
}