#pragma once

#include "athi_typedefs.h"

struct Athi_Rect
{
  std::string id;
  static constexpr u16 indices[]{0,1,2, 0,2,3};
  enum {POSITION_OFFSET, COLOR, NUM_UNIFORMS};
  enum {POSITION, INDICES, NUM_BUFFERS};
  u32 VAO;
  u32 shaderProgram;
  u32 uniform[NUM_UNIFORMS];

  vec2 pos;
  f32 width;
  f32 height;
  vec4 color;

  Athi_Rect() = default;
  void init()
  {
    shaderProgram  = glCreateProgram();
    const u32 vs   = createShader("./res/athi_rect_shader.vs", GL_VERTEX_SHADER);
    const u32 fs   = createShader("./res/athi_rect_shader.fs", GL_FRAGMENT_SHADER);

    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);

    glLinkProgram(shaderProgram);
    glValidateProgram(shaderProgram);
    validateShaderProgram("rect_constructor", shaderProgram);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    u32 VBO[NUM_BUFFERS];
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

    uniform[POSITION_OFFSET] = glGetUniformLocation(shaderProgram, "position_offset");
    uniform[COLOR]    = glGetUniformLocation(shaderProgram, "color");
  }

  void draw()
  {
    glBindVertexArray(VAO);
    glUseProgram(shaderProgram);
    glUniform2f(uniform[POSITION_OFFSET], pos.x, pos.y);
    glUniform4f(uniform[COLOR], color.r, color.g, color.g, color.a);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
  }
};
