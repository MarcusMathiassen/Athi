#include "athi_shader.h"

#include "athi_utility.h"  // read_file

#ifdef _WIN32
#include <sys/stat.h>
#else
// Not Windows? Assume unix-like.
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

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

Shader::~Shader() noexcept { glDeleteProgram(*program); }

void Shader::link() noexcept {
  glLinkProgram(*program);
  glValidateProgram(*program);
  validate_shader_program();

  for (auto & [ file_handle, shader ] : shaders) {
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

  // Check if any shaders need reloading
  for (auto & [ file_handle, shader ] : shaders) {
    const auto timestamp = GetShaderFileTimestamp(file_handle.file.c_str());
    if (timestamp > file_handle.timestamp) {
      need_to_reload = true;
      file_handle.timestamp = timestamp;
    }
  }

  // @Performance: You should only need to reload the changed shader.
  // .. If so, reload the shader, and for now, all its friends.
  if (need_to_reload) {
    glDeleteProgram(*program);
    *program = glCreateProgram();
    for (auto & [ file_handle, shader ] : shaders) {
      console->info("Reloading shader: {}", file_handle.file);
      shader = create_shader(file_handle.file, file_handle.shader_type);
      glAttachShader(*program, shader);
    }

    // We have to rebind the attrib locations in case they are out of sync.
    for (auto & [ name, integer ] : attribs) {
      glBindAttribLocation(*program, integer, name.c_str());
    }
    link();
  }
}

void Shader::init(std::string_view name) noexcept {
  this->name = name;
  program = std::make_unique<u32>();
  *program = glCreateProgram();
}

void Shader::load_from_file(const string& file,
                            const ShaderType shader_type) noexcept {
  const auto file_name = shader_folder_path + file;
  auto shader = create_shader(file_name, shader_type);
  glAttachShader(*program, shader);

  const auto timestamp = GetShaderFileTimestamp(file_name.c_str());
  FileHandle file_handle{file_name, shader_type, timestamp};

  shaders.emplace_back(std::tuple<FileHandle, u32>{file_handle, shader});

  console->info("Shader loaded: {}", file_name);
}

void Shader::bind_attrib(const char* name) noexcept {
  const auto integer = static_cast<s32>(attribs.size());
  glBindAttribLocation(*program, integer, name);
  attribs[name] = integer;
}

void Shader::add_uniform(const string& name) noexcept {
  if constexpr (ONLY_RUNS_IN_DEBUG_MODE) {
    if (!is_linked) {
      console->warn("[{}] add_uniform({}) called before shader was linked.",
                    this->name, name);
    }
  }
  uniforms[name] = glGetUniformLocation(*program, name.c_str());
}

u32 Shader::get_attrib(const string& name) const noexcept {
  return attribs.at(name);
}

void Shader::set_uniform(const string& name, float x, float y) const noexcept {
  glUniform2f(uniforms.at(name), x, y);
}

void Shader::set_uniform(const string& name, float x, float y, float z) const
    noexcept {
  glUniform3f(uniforms.at(name), x, y, z);
}
void Shader::set_uniform(const string& name, const vec2& v) const noexcept {
  glUniform2f(uniforms.at(name), v.x, v.y);
}
void Shader::set_uniform(const string& name, const vec3& v) const noexcept {
  glUniform3f(uniforms.at(name), v.x, v.y, v.z);
}
void Shader::set_uniform(const string& name, const vec4& v) const noexcept {
  glUniform4f(uniforms.at(name), v.x, v.y, v.z, v.w);
}
void Shader::set_uniform(const string& name, const mat4& m) const noexcept {
  glUniformMatrix4fv(uniforms.at(name), 1, GL_FALSE, &m[0][0]);
}
void Shader::set_uniform(const string& name, const mat3& m) const noexcept {
  glUniformMatrix3fv(uniforms.at(name), 1, GL_FALSE, &m[0][0]);
}
void Shader::set_uniform(const string& name, float val) const noexcept {
  glUniform1f(uniforms.at(name), val);
}
void Shader::set_uniform(const string& name, int val) const noexcept {
  glUniform1i(uniforms.at(name), val);
}
void Shader::set_uniform(const string& name, bool val) const noexcept {
  glUniform1i(uniforms.at(name), val);
}

void Shader::validate_shader(const string& file, const char* type,
                             u32 shader) const noexcept {
  char infoLog[512] = {0};
  s32 success;
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
  s32 success;
  glGetProgramiv(*program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(*program, sizeof(infoLog), NULL, infoLog);
    console->warn("\nERROR::SHADER::PROGRAM::{}::LINKING::FAILED\n\n{}", name,
                  infoLog);
  }
}

u32 Shader::create_shader(const string& file, ShaderType shader_type) const
    noexcept {
  char* source = NULL;
  read_file(file.c_str(), &source);

  u32 shader{0};

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
