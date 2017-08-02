#include "athi_text.h"

std::vector<Athi_Text*> text_buffer;

void Athi_Text::init()
{
  pos.x -= DIST_BETW_CHAR*0.5f; // looks better this way
}

Athi_Text_Manager::~Athi_Text_Manager()
{
  glDeleteVertexArrays(1, &VAO);
}

void Athi_Text_Manager::draw() const
{
  glBindVertexArray(VAO);
  glUseProgram(shader_program);
  texture.bind(0);
  
  const f32 inverse_aspect = 1.0f / (f32)camera.aspect_ratio;
  
  for (const auto &text: text_buffer)
  {
    Transform temp{vec3(text->pos,0), vec3(), vec3(1,1,1)};
    temp.scale = vec3(inverse_aspect, 1, 0);
    
    glUniform4f(uniform[COLOR], text->color.r, text->color.g, text->color.b, text->color.a);
    const size_t num_chars{text->str.length()};
    for (size_t i = 0; i < num_chars; ++i)
    {
      if (text->str[i] == ' ') continue;
      temp.pos.x = text->pos.x + DIST_BETW_CHAR * i * inverse_aspect;
      mat4 trans = temp.get_model();
      
      glUniformMatrix4fv(uniform[TRANSFORM], 1, GL_FALSE, &trans[0][0]);
      glUniform1i(uniform[TEXTCOORD_INDEX], text->str[i]);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    }
  }
}

void Athi_Text_Manager::init()
{
  texture = Texture("../Resources/font_custom.png", GL_LINEAR);
  shader_program  = glCreateProgram();
  const u32 vs   = createShader("../Resources/text_shader.vs", GL_VERTEX_SHADER);
  const u32 fs   = createShader("../Resources/text_shader.fs", GL_FRAGMENT_SHADER);
  
  glAttachShader(shader_program, vs);
  glAttachShader(shader_program, fs);
  
  glLinkProgram(shader_program);
  glValidateProgram(shader_program);
  validateShaderProgram("text_manager", shader_program);
  
  glDetachShader(shader_program, vs);
  glDetachShader(shader_program, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);
  
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  
  u32 VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  
  uniform[TRANSFORM]       = glGetUniformLocation(shader_program, "transform");
  uniform[COLOR]           = glGetUniformLocation(shader_program, "color");
  uniform[TEXTCOORD_INDEX] = glGetUniformLocation(shader_program, "textCoord_index");
}

void init_text_manager()
{
  athi_text_manager.init();
}

void add_text(Athi_Text* text)
{
  text->init();
  text_buffer.emplace_back(text);
}

void draw_all_text()
{
  athi_text_manager.draw();
}
