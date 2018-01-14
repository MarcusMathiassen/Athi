#pragma once

#include "athi_typedefs.h"
#include "athi_settings.h"

#include <vector>
#include <unordered_map>
#include <string>

#define GLEW_STATIC
#include <GL/glew.h>

class Shader {
private:
  std::string name;
  std::uint32_t program;
  std::vector<std::uint32_t> shaders;
  std::unordered_map<std::string, std::int32_t> uniforms;

public:
  Shader() = default;
  ~Shader();
  void bind() const noexcept;
  void init(std::string_view name) noexcept;
  void load_from_file(const std::string& file) noexcept;
  void bind_attrib(const char* name) noexcept;
  std::uint32_t get_attrib(const std::string& name) const noexcept;
  void link();

  void setUniform(const std::string &name, float x,float y,float z) const noexcept;
  void setUniform(const std::string &name, const glm::vec3 & v) const noexcept;
  void setUniform(const std::string &name, const glm::vec4 & v) const noexcept;
  void setUniform(const std::string &name, const glm::mat4 & m) const noexcept;
  void setUniform(const std::string &name, const glm::mat3 & m) const noexcept;
  void setUniform(const std::string &name, float val) const noexcept;
  void setUniform(const std::string &name, int val) const noexcept;
  void setUniform(const std::string &name, bool val) const noexcept;
};

void validateShader(const char* file, const char* type, std::uint32_t shader) noexcept;
void validateShaderProgram(const char* name, std::uint32_t shaderProgram) noexcept;
std::uint32_t createShader(const char* file, const GLenum type) noexcept;
