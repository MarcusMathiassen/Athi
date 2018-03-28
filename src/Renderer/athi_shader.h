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

#include <GL/glew.h>

#include <vector>  // std::vector
#include <memory>  // std::unique_ptr
#include <string>  // std::string
#include <unordered_map>  // std::unordered_map

#include <glm/vec2.hpp> // glm::vec2
#include <glm/vec2.hpp> // glm::vec3
#include <glm/vec2.hpp> // glm::vec4
#include <glm/mat3x3.hpp> // glm::mat3
#include <glm/mat4x4.hpp> // glm::mat4

using std::vector;
using std::string;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;


class Shader
{
private:
  enum class shader_type
  {
    vertex    = GL_VERTEX_SHADER,
    fragment  = GL_FRAGMENT_SHADER,
    geometry  = GL_GEOMETRY_SHADER,
    compute   = GL_COMPUTE_SHADER,
  };

  struct FileHandle
  {
    string        source;
    shader_type   shader_type;
    double        last_write_time;
  };

  string  name;
  bool    is_linked   {false};

  vector<std::tuple<FileHandle, GLuint>>    shaders;
  std::unordered_map<string, GLuint>        uniforms_map;
  std::unordered_map<string, GLuint>        attribs_map;
  vector<std::tuple<string, string>>        preambles_storage;

  static constexpr const char* shader_folder_path{"../Resources/Shaders/"};
public:

  GLuint          program;

  vector<string>  sources;
  vector<string>  preambles;
  vector<string>  attribs;
  vector<string>  uniforms;

  ~Shader();

  GLuint create_shader(const string& file, const shader_type type) const noexcept;

  void reload() noexcept;
  void link() noexcept;
  void bind() noexcept;

  void finish() noexcept;

  void set_uniform(const string& name, GLfloat x, GLfloat y) const noexcept;
  void set_uniform(const string& name, GLfloat x, GLfloat y, GLfloat z) const noexcept;
  void set_uniform(const string& name, const vec2& v) const noexcept;
  void set_uniform(const string& name, const vec3& v) const noexcept;
  void set_uniform(const string& name, const vec4& v) const noexcept;
  void set_uniform(const string& name, const mat3& m) const noexcept;
  void set_uniform(const string& name, const mat4& m) const noexcept;
  void set_uniform(const string& name, GLfloat val) const noexcept;
  void set_uniform(const string& name, GLint val) const noexcept;
  void set_uniform(const string& name, GLuint val) const noexcept;
  void set_uniform(const string& name, GLboolean val) const noexcept;
};

void validate_shader(const string& file, const char* type, GLuint shader) noexcept;
void validate_shader_program(const string& name, GLuint program) noexcept;
