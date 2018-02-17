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

#include "athi_shader.h"

#include "../athi_utility.h"  // read_file
#include "opengl_utility.h"  //   check_gl_error();
#include "../athi_settings.h"

Shader::~Shader() {
  glDeleteProgram(program);
  check_gl_error();
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
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
    console->warn("\nERROR::SHADER::PROGRAM::{}::LINKING::FAILED\n\n{}", name,
                  infoLog);
  }
}

u32 Shader::create_shader(const string& file, shader_type type) const noexcept {

  vector<string> sources;
  sources.reserve(1 + preambles_storage.size());

  sources.emplace_back("\n#version 410\n");

  for (const auto & [file, source] : preambles_storage)
  {
    sources.emplace_back(source);
  }

  sources.emplace_back(get_content_of_file(file));

  // @Hack: this needs to be in a char* array for some reason
  std::vector<const char*> charVec(sources.size(),nullptr);
  for (u32 i = 0; i < sources.size(); ++i) {
      charVec[i]= sources[i].c_str();
  }

  u32 shader{0};

  switch (type) {
    case shader_type::vertex:
      shader = glCreateShader(GL_VERTEX_SHADER); check_gl_error();
      break;
    case shader_type::fragment:
      shader = glCreateShader(GL_FRAGMENT_SHADER);  check_gl_error();

      break;
    case shader_type::geometry:
      shader = glCreateShader(GL_GEOMETRY_SHADER);  check_gl_error();

      break;
    case shader_type::compute:
      shader = glCreateShader(GL_COMPUTE_SHADER);  check_gl_error();

      break;
  }

  if (!sources.empty()) {
    glShaderSource(shader, static_cast<GLsizei>(sources.size()), &charVec[0], NULL);
    check_gl_error();
  }

  glCompileShader(shader);
  check_gl_error();

  switch (type) {
    case shader_type::vertex:
      validate_shader(file, "Vertex", shader);
      break;
    case shader_type::fragment:
      validate_shader(file, "Fragment", shader);
      break;
    case shader_type::geometry:
      validate_shader(file, "Geometry", shader);
      break;
    case shader_type::compute:
      validate_shader(file, "Compute", shader);
      break;
  }

  return shader;
}

void Shader::link() noexcept {
  glLinkProgram(program);
  check_gl_error();
  glValidateProgram(program);
  check_gl_error();
  validate_shader_program();
  check_gl_error();

  for (auto & [ file_handle, shader ] : shaders) {
    glDetachShader(program, shader);
    check_gl_error();
    glDeleteShader(shader);
    check_gl_error();
  }
  check_gl_error();

  is_linked = true;

  for (u32 i = 0; i < uniforms.size(); ++i) {
    uniforms_map[uniforms[i]] = glGetUniformLocation(program, uniforms[i].c_str());
  }

  check_gl_error();
}

void Shader::bind() noexcept {
  if constexpr (DEBUG_MODE) reload();
  glUseProgram(program);
  check_gl_error();
}

void Shader::finish() noexcept {
  program = glCreateProgram();
  check_gl_error();


  // Read in all preambles
  for (const auto & file: preambles)
  {
    string source = get_content_of_file(shader_folder_path + file);
    preambles_storage.emplace_back(std::tuple<string, string>(shader_folder_path + file, source + "\n"));
    //console->info("Preamble loaded: {}", file);
  }

  for (u32 i = 0; i < sources.size(); ++i) {
    const auto source = shader_folder_path + sources[i];

    FileHandle file;
    file.source = source;
    file.last_write_time = GetShaderFileTimestamp(source.c_str());

    const auto ext = source.substr(source.rfind('.'));

    u32 shader = 0;
    if (ext == ".vert") {
      shader = create_shader(source, shader_type::vertex);
      file.shader_type = shader_type::vertex;
    } else if (ext == ".frag") {
      shader = create_shader(source, shader_type::fragment);
      file.shader_type = shader_type::fragment;
    } else if (ext == ".geom") {
      shader = create_shader(source, shader_type::geometry);
      file.shader_type = shader_type::geometry;
    } else if (ext == ".comp") {
      shader = create_shader(source, shader_type::compute);
      file.shader_type = shader_type::compute;
    }
    glAttachShader(program, shader);

    //console->info("Shader loaded: {}", sources[i]);
    shaders.emplace_back(file, shader);
  }
  check_gl_error();

  for (u32 i = 0; i < attribs.size(); ++i) {
    attribs_map[attribs[i]] = i;
    glBindAttribLocation(program, i, attribs[i].c_str());
  }
  check_gl_error();

  link();
}

void Shader::set_uniform(const string& name, f32 x, f32 y) const noexcept {
  glUniform2f(uniforms_map.at(name), x, y);
  check_gl_error();
}

void Shader::set_uniform(const string& name, f32 x, f32 y, f32 z) const
    noexcept {
  glUniform3f(uniforms_map.at(name), x, y, z);
  check_gl_error();
}
void Shader::set_uniform(const string& name, const vec2& v) const noexcept {
  glUniform2f(uniforms_map.at(name), v.x, v.y);
  check_gl_error();
}
void Shader::set_uniform(const string& name, const vec3& v) const noexcept {
  glUniform3f(uniforms_map.at(name), v.x, v.y, v.z);
  check_gl_error();
}
void Shader::set_uniform(const string& name, const vec4& v) const noexcept {
  glUniform4f(uniforms_map.at(name), v.x, v.y, v.z, v.w);
  check_gl_error();
}
void Shader::set_uniform(const string& name, const mat4& m) const noexcept {
  glUniformMatrix4fv(uniforms_map.at(name), 1, GL_FALSE, &m[0][0]);
  check_gl_error();
}
void Shader::set_uniform(const string& name, const mat3& m) const noexcept {
  glUniformMatrix3fv(uniforms_map.at(name), 1, GL_FALSE, &m[0][0]);
  check_gl_error();
}
void Shader::set_uniform(const string& name, f32 val) const noexcept {
  glUniform1f(uniforms_map.at(name), val);
  check_gl_error();
}
void Shader::set_uniform(const string& name, s32 val) const noexcept {
  glUniform1i(uniforms_map.at(name), val);
  check_gl_error();
}

void Shader::set_uniform(const string& name, u32 val) const noexcept {
  glUniform1i(uniforms_map.at(name), val);
  check_gl_error();
}

void Shader::set_uniform(const string& name, bool val) const noexcept {
  glUniform1i(uniforms_map.at(name), val);
  check_gl_error();
}

void Shader::reload() noexcept {
  bool need_to_reload = false;

  // Check if any shaders need reloading
  for (auto & [ file, shader ] : shaders) {
    const auto timestamp = GetShaderFileTimestamp(file.source.c_str());
    if (timestamp > file.last_write_time) {
      need_to_reload = true;
      file.last_write_time = timestamp;
    }
  }

  // @Performance: You should only need to reload the changed shader.
  // .. If so, reload the shader, and for now, all its friends.
  if (need_to_reload) {
    glDeleteProgram(program);
    check_gl_error();
    program = glCreateProgram();
    check_gl_error();
    for (auto & [ file, shader ] : shaders) {
      console->info("reloading shader: {}", file.source);
      shader = create_shader(file.source, file.shader_type);
      glAttachShader(program, shader);
      check_gl_error();
    }

    // We have to rebind the attrib locations in case they are out of sync.
    for (auto & [ name, integer ] : attribs_map) {
      glBindAttribLocation(program, integer, name.c_str());
    }
    check_gl_error();
    link();
    check_gl_error();
  }
}
