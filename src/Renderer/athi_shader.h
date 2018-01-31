#pragma once

#include "../athi_typedefs.h"

#include <unordered_map> // unordered_map
#include <memory> // unique_ptr

#define GLEW_STATIC
#include <GL/glew.h>


#ifdef _WIN32
#include <sys/stat.h>
#else
// Not Windows? Assume unix-like.
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

struct Shader {

  enum class shader_type {
    vertex = GL_VERTEX_SHADER,
    fragment = GL_FRAGMENT_SHADER,
    geometry = GL_GEOMETRY_SHADER,
    compute = GL_COMPUTE_SHADER,
  };

  struct FileHandle {
    string source;
    shader_type shader_type;
    f64 last_write_time;
  };

  string name;
  bool is_linked{false};
  u32 program;
  vector<std::tuple<FileHandle, u32>> shaders;
  std::unordered_map<string, u32> uniforms_map;
  std::unordered_map<string, u32> attribs_map;
  vector<string> sources;
  vector<string> attribs;
  vector<string> uniforms;

  static constexpr const char* shader_folder_path{"../Resources/Shaders/"};

  ~Shader();

  void validate_shader(const string& file, const char* type, u32 shader) const noexcept;
  void validate_shader_program() const noexcept;
  u32 create_shader(const string& file, const shader_type type) const noexcept;

  void reload() noexcept;
  void link() noexcept;
  void bind() noexcept;

  void finish() noexcept;

  void set_uniform(const string& name, f32 x, f32 y) const noexcept;
  void set_uniform(const string& name, f32 x, f32 y, f32 z) const noexcept;
  void set_uniform(const string& name, const vec2& v) const noexcept;
  void set_uniform(const string& name, const vec3& v) const noexcept;
  void set_uniform(const string& name, const vec4& v) const noexcept;
  void set_uniform(const string& name, const mat4& m) const noexcept;
  void set_uniform(const string& name, const mat3& m) const noexcept;
  void set_uniform(const string& name, f32 val) const noexcept;
  void set_uniform(const string& name, s32 val) const noexcept;
  void set_uniform(const string& name, bool val) const noexcept;
};

static u64 GetShaderFileTimestamp(const char* filename) noexcept {
  u64 timestamp = 0;

#ifdef _WIN32
  struct __stat64 stFileInfo;
  if (_stat64(filename, &stFileInfo) == 0) {
    timestamp = stFileInfo.st_mtime;
  }
#else
  struct stat fileStat;

  if (stat(filename, &fileStat) == -1) {
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