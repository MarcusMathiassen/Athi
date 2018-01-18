#include "athi_shader.h"
#include "athi_resource.h"
#include "athi_settings.h"
#include "athi_typedefs.h"
#include "athi_utility.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

#ifdef _WIN32
#include <sys/stat.h>
#else
// Not Windows? Assume unix-like.
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

Shader::~Shader() noexcept { glDeleteProgram(*program); }

void Shader::link() noexcept {
  glLinkProgram(*program);
  glValidateProgram(*program);
  validate_shader_program();

  for (auto & [file_handle, shader]: shaders) {
    glDetachShader(*program, shader);
    glDeleteShader(shader);
  }

  for (auto & [ name, integer ] : uniforms) {
    integer = glGetUniformLocation(*program, name.c_str());
  }

  is_linked = true;
}

void Shader::bind() noexcept { 
  if constexpr (ONLY_RUNS_IN_DEBUG_MODE) reload();
  glUseProgram(*program); 
}

void Shader::reload() noexcept {

  bool need_to_reload = false;

  // For every shader that needs reloading
  for (auto & [file_handle, shader]: shaders) {
      const auto timestamp = GetShaderFileTimestamp(file_handle.file.c_str());
      if (timestamp > file_handle.timestamp) {
        need_to_reload = true;
        file_handle.timestamp = timestamp;
      }
  }

  if (need_to_reload) {
    glDeleteProgram(*program);
    *program = glCreateProgram();
    for (auto & [file_handle, shader]: shaders) {
      console->info("Reloading shader: {}", file_handle.file);
      shader = create_shader(file_handle.file, file_handle.shader_type);
      glAttachShader(*program, shader);
    }

    for (auto & [ name, integer ] : attribs) {
      glBindAttribLocation(*program, integer, name.c_str());
    }
    link();
  }
}

void Shader::init(std::string_view name) noexcept {
  this->name = name;
  program = std::make_unique<std::uint32_t>();
  *program = glCreateProgram();
}

void Shader::load_from_file(const std::string& file,
                            const ShaderType shader_type) noexcept {
  auto shader = create_shader(file, shader_type);
  glAttachShader(*program, shader);
  
  const auto timestamp = GetShaderFileTimestamp(file.c_str());
  FileHandle file_handle{file, shader_type, timestamp};

  shaders.emplace_back(std::tuple<FileHandle, std::uint32_t>{file_handle, shader});
}

void Shader::bind_attrib(const char* name) noexcept {
  const auto integer = static_cast<std::int32_t>(attribs.size());
  glBindAttribLocation(*program, integer, name);
  attribs[name] = integer;
}

void Shader::add_uniform(const std::string& name) noexcept {

  if constexpr (ONLY_RUNS_IN_DEBUG_MODE) {
    if (!is_linked) {
        console->warn("[{}] add_uniform({}) called before shader was linked.",
                       this->name, name);
    }
  }
  uniforms[name] = glGetUniformLocation(*program, name.c_str());
}

std::uint32_t Shader::get_attrib(const std::string& name) const noexcept {
  return attribs.at(name);
}

void Shader::setUniform(const std::string& name, float x, float y) const
    noexcept {
  glUniform2f(uniforms.at(name), x, y);
}

void Shader::setUniform(const std::string& name, float x, float y,
                        float z) const noexcept {
  glUniform3f(uniforms.at(name), x, y, z);
}
void Shader::setUniform(const std::string& name, const glm::vec2& v) const
    noexcept {
  glUniform2f(uniforms.at(name), v.x, v.y);
}
void Shader::setUniform(const std::string& name, const glm::vec3& v) const
    noexcept {
  glUniform3f(uniforms.at(name), v.x, v.y, v.z);
}
void Shader::setUniform(const std::string& name, const glm::vec4& v) const
    noexcept {
  glUniform4f(uniforms.at(name), v.x, v.y, v.z, v.w);
}
void Shader::setUniform(const std::string& name, const glm::mat4& m) const
    noexcept {
  glUniformMatrix4fv(uniforms.at(name), 1, GL_FALSE, &m[0][0]);
}
void Shader::setUniform(const std::string& name, const glm::mat3& m) const
    noexcept {
  glUniformMatrix3fv(uniforms.at(name), 1, GL_FALSE, &m[0][0]);
}
void Shader::setUniform(const std::string& name, float val) const noexcept {
  glUniform1f(uniforms.at(name), val);
}
void Shader::setUniform(const std::string& name, int val) const noexcept {
  glUniform1i(uniforms.at(name), val);
}
void Shader::setUniform(const std::string& name, bool val) const noexcept {
  glUniform1i(uniforms.at(name), val);
}

void Shader::validate_shader(const std::string& file, const char* type,
                             std::uint32_t shader) const noexcept {
  char infoLog[512] = {0};
  std::int32_t success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
    console->warn("\nERROR::FILE {}\n", file);
    console->warn("ERROR::SHADER::{}::COMPILATION::FAILED\n\n{}", type,
                  infoLog);
  }
}

void Shader::validate_shader_program() const noexcept {
  char infoLog[512] = {0};
  std::int32_t success;
  glGetProgramiv(*program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(*program, sizeof(infoLog), NULL, infoLog);
    console->warn("\nERROR::SHADER::PROGRAM::{}::LINKING::FAILED\n\n{}", name,
                  infoLog);
  }
}

std::uint32_t Shader::create_shader(const std::string& file,
                                    ShaderType shader_type) const noexcept {
  char* source = NULL;
  read_file(file.c_str(), &source);

  std::uint32_t shader{0};

  switch (shader_type) {
    case ShaderType::Vertex:
      shader = glCreateShader(GL_VERTEX_SHADER);
      break;
    case ShaderType::Fragment:
      shader = glCreateShader(GL_FRAGMENT_SHADER);
      break;
    case ShaderType::Geometry:
      shader = glCreateShader(GL_GEOMETRY_SHADER);
      break;
    case ShaderType::Compute:
      shader = glCreateShader(GL_COMPUTE_SHADER);
      break;
  }

  if (NULL != source) {
    glShaderSource(shader, 1, &source, NULL);
    free(source);
  }

  glCompileShader(shader);

  switch (shader_type) {
    case ShaderType::Vertex:
      validate_shader(file, "Vertex", shader);
      break;
    case ShaderType::Fragment:
      validate_shader(file, "Fragment", shader);
      break;
    case ShaderType::Geometry:
      validate_shader(file, "Geometry", shader);
      break;
    case ShaderType::Compute:
      validate_shader(file, "Compute", shader);
      break;
  }

  return shader;
}
