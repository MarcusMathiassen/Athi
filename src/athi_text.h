#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include <iostream>

#include "athi_typedefs.h"
#include "athi_texture.h"
#include "athi_utility.h"
#include <vector>
#include <memory>

#define TOP     0.85f
#define BOTTOM -1.0f
#define LEFT   -1.0f
#define RIGHT   1.0f

#define ROW 0.06f
#define DIST_BETW_CHAR 0.06125f * 0.5f

#define FONT_ATLAS_CHARACTERS_ROWS  16
#define FONT_ATLAS_CHARACTERS_COLS  16
#define FONT_ATLAS_CHARACTER_WIDTH  0.06125f
#define FONT_ATLAS_CHARACTER_HEIGHT 0.06125f

struct Athi_Text
{
  std::string id;
  s32*   int_dynamic_part{nullptr};
  f32*   float_dynamic_part{nullptr};
  vec2 pos {0.0f, 0.0f};
  std::string str{"default text"};
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Athi_Text() = default;
};


struct Athi_Text_Manager
{
  std::string id;
  static constexpr u16 indices[]{0,1,2, 0,2,3};
  enum { POSITION_OFFSET, COLOR, TEXTCOORD_INDEX, NUM_UNIFORMS};
  u32           VAO;
  u32           shaderProgram;
  u32           uniform[NUM_UNIFORMS];
  Texture       texture;
  std::string   font_atlas_path;

  std::vector<std::unique_ptr<Athi_Text> >   text_buffer;

  Athi_Text_Manager() = default;
  ~Athi_Text_Manager()
  {
    glDeleteVertexArrays(1, &VAO);
  }

  void draw() const
  {
    glBindVertexArray(VAO);
    glUseProgram(shaderProgram);
    texture.bind(0);
    for (const auto &text: text_buffer)
    {
      std::string temp = text->str;
      if (text->float_dynamic_part != nullptr) temp += std::to_string(*text->float_dynamic_part);
      if (text->int_dynamic_part != nullptr)temp += std::to_string(*text->int_dynamic_part);

      glUniform4f(uniform[COLOR], text->color.r, text->color.g, text->color.g, text->color.a);
      const size_t num_chars{temp.length()};
      for (size_t i = 0; i < num_chars; ++i)
      {
        if (temp[i] == ' ') continue;
        glUniform2f(uniform[POSITION_OFFSET], text->pos.x + i * DIST_BETW_CHAR, text->pos.y);
        glUniform1i(uniform[TEXTCOORD_INDEX], temp[i]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
      }
    }
  }

  void init()
  {
    texture = Texture(font_atlas_path.c_str(), GL_LINEAR);
    shaderProgram  = glCreateProgram();
    const u32 vs   = createShader("./res/text_shader.vs", GL_VERTEX_SHADER);
    const u32 fs   = createShader("./res/text_shader.fs", GL_FRAGMENT_SHADER);

    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);

    glLinkProgram(shaderProgram);
    glValidateProgram(shaderProgram);
    validateShaderProgram("text_manager_init", shaderProgram);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    u32 VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    uniform[POSITION_OFFSET] = glGetUniformLocation(shaderProgram, "pos_offset");
    uniform[COLOR]           = glGetUniformLocation(shaderProgram, "color");
    uniform[TEXTCOORD_INDEX] = glGetUniformLocation(shaderProgram, "textCoord_index");
  }
};

template <typename T>
auto create_text(T* t)
{
  auto text = std::make_unique<Athi_Text>();
  return text;
}
