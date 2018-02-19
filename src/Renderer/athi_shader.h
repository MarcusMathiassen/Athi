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

#include "../athi_typedefs.h"

#include <memory>         // unique_ptr
#include <unordered_map>  // unordered_map

#include <GL/glew.h>

#ifdef _WIN32
#include <sys/stat.h>
#else
// Not Windows? Assume unix-like.
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

class Shader {
private:
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
  vector<std::tuple<FileHandle, u32>> shaders;
  std::unordered_map<string, u32> uniforms_map;
  std::unordered_map<string, u32> attribs_map;
  std::vector<std::tuple<string, string>> preambles_storage;

  static constexpr const char* shader_folder_path{"./Resources/Shaders/"};
public:

  u32 program;
  vector<string> sources;
  vector<string> preambles;
  vector<string> attribs;
  vector<string> uniforms;


  ~Shader();

  void validate_shader(const string& file, const char* type, u32 shader) const
      noexcept;
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
  void set_uniform(const string& name, u32 val) const noexcept;
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
