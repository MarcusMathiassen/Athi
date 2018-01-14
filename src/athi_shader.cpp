#include "athi_shader.h"
#include "athi_typedefs.h"
#include "athi_settings.h"
#include "athi_utility.h"

#include <vector>
#include <unordered_map>
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>

void Shader::init() {
  program = glCreateProgram();
}

void Shader::load_from_file(const std::string &file)
{
  const auto type = file.substr(file.length()-3, file.length());

  if (type == ".vs") {
    auto shader = (createShader(file.c_str(), GL_VERTEX_SHADER));
    glAttachShader(program, shader);
    shaders.emplace_back(shader);
  }
  else if (type == ".fs") {
    auto shader = (createShader(file.c_str(), GL_FRAGMENT_SHADER));
    glAttachShader(program, shader);
    shaders.emplace_back(shader);
  }
  else if (type == ".gs") {
    auto shader = (createShader(file.c_str(), GL_GEOMETRY_SHADER));
    glAttachShader(program, shader);
    shaders.emplace_back(shader);
  }
  else if (type == ".cs") {
    auto shader = (createShader(file.c_str(), GL_COMPUTE_SHADER));
    glAttachShader(program, shader);
    shaders.emplace_back(shader);
  }
  else {
    console->warn("not a valid shader type. file: {}", file);
  }
}

void Shader::bind_attrib_loc(const char* name) {
  const auto integer = static_cast<std::int32_t>(uniforms.size());
  glBindAttribLocation(program, integer, name);
  uniforms[name] = integer;
}

void Shader::link() {
  glLinkProgram(program);
  glValidateProgram(program);
  validateShaderProgram("shader", program);

  for (auto& shader: shaders
    ) {
    glDetachShader(program, shader);
    glDeleteShader(shader);
  }
}

void validateShader(const char *file, const char *type, std::uint32_t shader) {
  char infoLog[512] = {0};
  std::int32_t success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
    printf("\nERROR::FILE %s\n", file);
    printf("ERROR::SHADER::%s::COMPILATION::FAILED\n\n%s", type, infoLog);
  }
}

void validateShaderProgram(const char *name, std::uint32_t shaderProgram) {
  char infoLog[512] = {0};
  std::int32_t success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
    printf("\nERROR::SHADER::PROGRAM::%s::LINKING::FAILED\n\n%s", name, infoLog);
  }
}

std::uint32_t createShader(const char *file, const GLenum type) {
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
  return shader;
}
