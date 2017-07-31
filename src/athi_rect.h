#pragma once

#include "athi_typedefs.h"

#include "athi_utility.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct Athi_Rect
{
  string id;
  static constexpr u16 indices[]{0,1,2, 0,2,3};
  enum {POSITION_OFFSET, COLOR, NUM_UNIFORMS};
  enum {POSITION, INDICES, NUM_BUFFERS};

  u32 VAO;
  u32 VBO[NUM_BUFFERS];
  u32 shader_program;
  u32 uniform[NUM_UNIFORMS];

  vec2 pos;
  f32 width;
  f32 height;
  vec4 color;

  Athi_Rect() = default;
  ~Athi_Rect()
  {
    glDeleteBuffers(NUM_BUFFERS, VBO);
    glDeleteVertexArrays(1, &VAO);
  }
  void init()
  {
    shader_program  = glCreateProgram();
    const u32 vs   = createShader("../Resources/athi_rect_shader.vs", GL_VERTEX_SHADER);
    const u32 fs   = createShader("../Resources/athi_rect_shader.fs", GL_FRAGMENT_SHADER);

    glAttachShader(shader_program, vs);
    glAttachShader(shader_program, fs);

    glBindAttribLocation(shader_program, 0, "position");

    glLinkProgram(shader_program);
    glValidateProgram(shader_program);
    validateShaderProgram("rect_constructor", shader_program);

    glDetachShader(shader_program, vs);
    glDetachShader(shader_program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(NUM_BUFFERS, VBO);

    const f32 positions[] =
    {
      0.0f,  height,
      0.0f,  0.0f,
      width, 0.0f,
      width, height,
    };

    glBindBuffer( GL_ARRAY_BUFFER, VBO[POSITION] );
    glBufferData( GL_ARRAY_BUFFER, 4*sizeof(f32)*2, positions, GL_STATIC_DRAW );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[INDICES]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    uniform[POSITION_OFFSET] = glGetUniformLocation(shader_program, "position_offset");
    uniform[COLOR]           = glGetUniformLocation(shader_program, "color");
  }

  void draw() const
  {
    glBindVertexArray(VAO);
    glUseProgram(shader_program);
    glUniform2f(uniform[POSITION_OFFSET], pos.x, pos.y);
    glUniform4f(uniform[COLOR], color.r, color.g, color.g, color.a);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
  }
};
