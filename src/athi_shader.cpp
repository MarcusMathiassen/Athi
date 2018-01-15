#include "athi_shader.h"
#include "athi_typedefs.h"
#include "athi_settings.h"
#include "athi_utility.h"
#include "athi_resource.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>

#define GLEW_STATIC
#include <GL/glew.h>

Shader::~Shader() {
  glDeleteProgram(program);
}

void Shader::bind() const noexcept {
  glUseProgram(program);
}

void Shader::init(std::string_view name) noexcept {
  this->name = name;
  program = glCreateProgram();
}

void Shader::load_from_file(const std::string& file) noexcept
{
  const auto type = file.substr(file.length()-3, file.length());

  if (type == ".vs") {
    auto shader = createShader(file.c_str(), GL_VERTEX_SHADER);
    glAttachShader(program, shader);
    shaders.emplace_back(shader);
  }
  else if (type == ".fs") {
    auto shader = createShader(file.c_str(), GL_FRAGMENT_SHADER);
    glAttachShader(program, shader);
    shaders.emplace_back(shader);
  }
  else if (type == ".gs") {
    auto shader = createShader(file.c_str(), GL_GEOMETRY_SHADER);
    glAttachShader(program, shader);
    shaders.emplace_back(shader);
  }
  else if (type == ".cs") {
    auto shader = createShader(file.c_str(), GL_COMPUTE_SHADER);
    glAttachShader(program, shader);
    shaders.emplace_back(shader);
  }
  else {
    console->warn("not a valid shader type. file: {}", file.c_str());
  }
}

void Shader::bind_attrib(const char* name) noexcept {
  const auto integer = static_cast<std::int32_t>(uniforms.size());
  glBindAttribLocation(program, integer, name);
  uniforms[name] = integer;
}

std::uint32_t Shader::get_attrib(const std::string &name) const noexcept {
  return uniforms.at(name);
}

void Shader::link() {
  glLinkProgram(program);
  glValidateProgram(program);
  validateShaderProgram(name.c_str(), program);

  for (auto& shader: shaders) {
    glDetachShader(program, shader);
    glDeleteShader(shader);
  }

  for (auto & [name, integer]: uniforms) {
    integer = glGetUniformLocation(program, name.c_str());
  }
}


void Shader::setUniform(const std::string &name, float x,float y,float z) const noexcept
{
  glUniform3f(uniforms.at(name), x, y, z);
}
void Shader::setUniform(const std::string &name, const glm::vec3& v) const noexcept
{
  glUniform3f(uniforms.at(name), v.x, v.y, v.z);
}
void Shader::setUniform(const std::string &name, const glm::vec4& v) const noexcept
{
  glUniform4f(uniforms.at(name), v.x, v.y, v.z, v.w);
}
void Shader::setUniform(const std::string &name, const glm::mat4& m) const noexcept
{
  glUniformMatrix4fv(uniforms.at(name), 1, GL_FALSE, &m[0][0]);
}
void Shader::setUniform(const std::string &name, const glm::mat3& m) const noexcept
{
  glUniformMatrix3fv(uniforms.at(name), 1, GL_FALSE, &m[0][0]);
}
void Shader::setUniform(const std::string &name, float val) const noexcept
{
  glUniform1f(uniforms.at(name), val);
}
void Shader::setUniform(const std::string &name, int val) const noexcept
{
  glUniform1i(uniforms.at(name), val);
}
void Shader::setUniform(const std::string &name, bool val) const noexcept
{
  glUniform1i(uniforms.at(name), val);
}



void validateShader(const char* file, const char* type, std::uint32_t shader) noexcept {
  char infoLog[512] = {0};
  std::int32_t success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
    console->warn("\nERROR::FILE {}\n", file);
    console->warn("ERROR::SHADER::{}::COMPILATION::FAILED\n\n{}", type, infoLog);
  }
}

void validateShaderProgram(const char* name, std::uint32_t shaderProgram) noexcept {
  char infoLog[512] = {0};
  std::int32_t success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
    console->warn("\nERROR::SHADER::PROGRAM::{}::LINKING::FAILED\n\n{}", name, infoLog);
  }
}

std::uint32_t createShader(const char* file, const GLenum type) noexcept {

  if (auto resource = resource_manager.get_resource(file); resource)
    return resource;

  char *source = NULL;
  read_file(file, &source);


  std::uint32_t shader = glCreateShader(type);
  if (NULL != source) {
    glShaderSource(shader, 1, &source, NULL);
    free(source);
  }

  glCompileShader(shader);

  switch (type) {
  case GL_VERTEX_SHADER:
    validateShader(file, "VERTEX", shader);
    break;
  case GL_FRAGMENT_SHADER:
    validateShader(file, "FRAGMENT", shader);
    break;
  case GL_GEOMETRY_SHADER:
    validateShader(file, "GEOMETRY", shader);
    break;
  }

  resource_manager.add_resource(file, shader);
  return shader;
}
