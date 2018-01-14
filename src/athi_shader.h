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
  std::uint32_t program;
  std::vector<std::uint32_t> shaders;
  std::unordered_map<std::string, std::int32_t> uniforms;

public:
  void init();
  void load_from_file(const std::string &file);
  void bind_attrib_loc(const char* name);
  void link();
};

void validateShader(const char *file, const char *type, std::uint32_t shader);
void validateShaderProgram(const char *name, std::uint32_t shaderProgram);
std::uint32_t createShader(const char *file, const GLenum type);
