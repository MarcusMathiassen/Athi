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

void Athi_Text_Manager::draw()
{
  if (text_buffer.empty()) return;

  glBindVertexArray(VAO);
  glUseProgram(shader_program);
  texture.bind(0);

  const f32 inverse_aspect = 1.0f / (f32)camera.aspect_ratio;

  for (const auto &text: text_buffer)
  {
    // Setup local variables
    const std::string text_str  = text->str;
    const vec2 text_position    = text->pos;
    const vec4 text_color       = text->color;

    // Scale by the inverse aspectratio to make sure text is renderered correctly.
    Transform temp{vec3(text_position,0), vec3(), vec3(1,1,1)};
    temp.scale = vec3(inverse_aspect, 1, 0);

    // Assign color of text
    glUniform4f(uniform[COLOR], text_color.r, text_color.g, text_color.b, text_color.a);

    // Loop through characters, spacing them accordingly
    const size_t num_characters = text_str.length();
    for (size_t i = 0; i < num_characters; ++i)
    {
      if (text_str[i] == ' ') continue;
      temp.pos.x = text_position.x + DIST_BETW_CHAR * i * inverse_aspect;
      mat4 trans = temp.get_model();
      glUniformMatrix4fv(uniform[TRANSFORM], 1, GL_FALSE, &trans[0][0]);
      glUniform1i(uniform[TEXTCOORD_INDEX], text_str[i]);
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

  uniform[TRANSFORM]       = glGetUniformLocation(shader_program, "transform");
  uniform[COLOR]           = glGetUniformLocation(shader_program, "color");
  uniform[TEXTCOORD_INDEX] = glGetUniformLocation(shader_program, "textCoord_index");

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(NUM_BUFFERS, VBO);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[INDICES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
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
