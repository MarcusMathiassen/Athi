#pragma once

#include "athi_settings.h"

#include <string>
#include <unordered_map>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

enum class ShaderType {
  Vertex = GL_VERTEX_SHADER,
  Fragment = GL_FRAGMENT_SHADER,
  Geometry = GL_GEOMETRY_SHADER,
  Compute = GL_COMPUTE_SHADER,
};

struct FileHandle {
  std::string file;
  ShaderType shader_type;
  std::uint64_t timestamp{0};
};

class Shader {
 private:
  bool is_linked{false};
  std::string name;
  std::unique_ptr<std::uint32_t> program;
  std::vector<std::tuple<FileHandle, std::uint32_t>> shaders;
  std::unordered_map<std::string, std::int32_t> attribs;
  std::unordered_map<std::string, std::int32_t> uniforms;

  void validate_shader(const std::string &file, const char *type,
                       std::uint32_t shader) const noexcept;
  void validate_shader_program() const noexcept;
  std::uint32_t create_shader(const std::string &file,
                              const ShaderType shader_type) const noexcept;

 public:
  Shader() noexcept = default;
  ~Shader() noexcept;
  void link() noexcept;
  void reload() noexcept;
  void bind() noexcept;
  void init(std::string_view name) noexcept;
  void load_from_file(const std::string &file,
                      const ShaderType shader_type) noexcept;
  void bind_attrib(const char *name) noexcept;
  void add_uniform(const std::string &name) noexcept;
  std::uint32_t get_attrib(const std::string &name) const noexcept;

  void setUniform(const std::string &name, float x, float y) const noexcept;
  void setUniform(const std::string &name, float x, float y, float z) const noexcept;
  void setUniform(const std::string &name, const glm::vec2 &v) const noexcept;
  void setUniform(const std::string &name, const glm::vec3 &v) const noexcept;
  void setUniform(const std::string &name, const glm::vec4 &v) const noexcept;
  void setUniform(const std::string &name, const glm::mat4 &m) const noexcept;
  void setUniform(const std::string &name, const glm::mat3 &m) const noexcept;
  void setUniform(const std::string &name, float val) const noexcept;
  void setUniform(const std::string &name, int val) const noexcept;
  void setUniform(const std::string &name, bool val) const noexcept;
};

static uint64_t GetShaderFileTimestamp(const char* filename)
{
    uint64_t timestamp = 0;

#ifdef _WIN32
    struct __stat64 stFileInfo;
    if (_stat64(filename, &stFileInfo) == 0)
    {
        timestamp = stFileInfo.st_mtime;
    }
#else
    struct stat fileStat;

    if (stat(filename, &fileStat) == -1)
    {
        perror(filename);
        return 0;
    }

#ifdef __APPLE__
    timestamp = fileStat.st_mtimespec.tv_sec;
#else
    timestamp = fileStat.st_mtime;
#endif
#endif

    return timestamp;
}