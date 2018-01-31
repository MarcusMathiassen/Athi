#pragma once

#include "athi_typedefs.h"

#include <unordered_map>

#define GLEW_STATIC
#include <GL/glew.h>

enum class ShaderType {
  Vertex = GL_VERTEX_SHADER,
  Fragment = GL_FRAGMENT_SHADER,
  Geometry = GL_GEOMETRY_SHADER,
  Compute = GL_COMPUTE_SHADER,
};

struct FileHandle {
  string file;
  ShaderType shader_type;
  u64 timestamp{0};
};

class Shader {
 private:
  bool is_linked{false};
  string name;
  std::unique_ptr<u32> program;
  vector<std::tuple<FileHandle, u32>> shaders;
  std::unordered_map<string, s32> attribs;
  std::unordered_map<string, s32> uniforms;

  void validate_shader(const string& file, const char* type, u32 shader) const
      noexcept;
  void validate_shader_program() const noexcept;
  u32 create_shader(const string& file, const ShaderType shader_type) const
      noexcept;

  static constexpr const char* shader_folder_path{"../Resources/Shaders/"};

 public:
  Shader() noexcept = default;
  ~Shader() noexcept;
  void link() noexcept;
  void reload() noexcept;
  void bind() noexcept;
  void init(std::string_view name) noexcept;
  void load_from_file(const string& file,
                      const ShaderType shader_type) noexcept;
  void bind_attrib(const char* name) noexcept;
  void add_uniform(const string& name) noexcept;
  u32 get_attrib(const string& name) const noexcept;

  void setUniform(const string& name, float x, float y) const noexcept;
  void setUniform(const string& name, float x, float y, float z) const noexcept;
  void setUniform(const string& name, const vec2& v) const noexcept;
  void setUniform(const string& name, const vec3& v) const noexcept;
  void setUniform(const string& name, const vec4& v) const noexcept;
  void setUniform(const string& name, const mat4& m) const noexcept;
  void setUniform(const string& name, const mat3& m) const noexcept;
  void setUniform(const string& name, float val) const noexcept;
  void setUniform(const string& name, int val) const noexcept;
  void setUniform(const string& name, bool val) const noexcept;
};

static u64 GetShaderFileTimestamp(const char* filename) noexcept;