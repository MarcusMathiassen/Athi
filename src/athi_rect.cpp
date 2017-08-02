
#include "athi_rect.h"
#include "athi_circle.h"
#include "athi_camera.h"
#include "athi_transform.h"

#include "athi_utility.h"

std::vector<Athi_Rect*> rect_buffer;
Athi_Rect_Manager athi_rect_manager;

bool Rect::contains(u32 id)
{
  const vec2 o  = circle_buffer[id]->pos;
  const f32  r  = circle_buffer[id]->radius;
  if (o.x - r < max.x && o.x + r > min.x && o.y - r < max.y && o.y + r > min.y) return true;
  return false;
}

Athi_Rect_Manager::~Athi_Rect_Manager()
{
  glDeleteBuffers(NUM_BUFFERS, VBO);
  glDeleteVertexArrays(1, &VAO);
}

void Athi_Rect_Manager::init()
{
  shader_program  = glCreateProgram();
  const u32 vs    = createShader("../Resources/athi_rect_shader.vs", GL_VERTEX_SHADER);
  const u32 fs    = createShader("../Resources/athi_rect_shader.fs", GL_FRAGMENT_SHADER);

  glAttachShader(shader_program, vs);
  glAttachShader(shader_program, fs);

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

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[INDICES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  uniform[TRANSFORM] = glGetUniformLocation(shader_program, "transform");
  uniform[COLOR]     = glGetUniformLocation(shader_program, "color");
}

void Athi_Rect_Manager::draw()
{
  glBindVertexArray(VAO);
  glUseProgram(shader_program);

  const f32 inverse_aspect = 1.0f / (f32)camera.aspect_ratio;

  for (const auto &rect: rect_buffer)
  {
    Transform temp{vec3(rect->pos,0), vec3(), vec3(1,1,1)};
    temp.scale = vec3(rect->width * inverse_aspect, rect->height, 0);

    mat4 trans = temp.get_model();

    glUniform4f(uniform[COLOR], rect->color.r, rect->color.g, rect->color.b, rect->color.a);
    glUniformMatrix4fv(uniform[TRANSFORM], 1, GL_FALSE, &trans[0][0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
  }
}

void init_rect_manager()
{
  athi_rect_manager.init();
}

void add_rect(Athi_Rect* rect)
{
  rect_buffer.emplace_back(rect);
}

void draw_rect(const vec2& min, const vec2& max, const vec4& color, GLenum draw_type)
{
  glBindVertexArray(athi_rect_manager.VAO);
  glUseProgram(athi_rect_manager.shader_program);

  Transform temp{vec3(min,0), vec3(), vec3(1,1,1)};
  //const f32 inverse_aspect = 1.0f / (f32)camera.aspect_ratio;

  const f32 width = max.x - min.x;
  const f32 height = max.y - min.y;
  temp.scale = vec3(width, height, 0);
  mat4 trans = temp.get_model();

  glUniform4f(athi_rect_manager.uniform[athi_rect_manager.COLOR], color.r, color.g, color.b, color.a);
  glUniformMatrix4fv(athi_rect_manager.uniform[athi_rect_manager.TRANSFORM], 1, GL_FALSE, &trans[0][0]);
  glDrawElements(draw_type, 6, GL_UNSIGNED_SHORT, NULL);
}

void draw_rect(const vec2& min, f32 width, f32 height, const vec4& color, GLenum draw_type)
{
  glBindVertexArray(athi_rect_manager.VAO);
  glUseProgram(athi_rect_manager.shader_program);

  Transform temp{vec3(min,0), vec3(), vec3(1,1,1)};
  //const f32 inverse_aspect = 1.0f / (f32)camera.aspect_ratio;

  temp.scale = vec3(width, height, 0);
  mat4 trans = temp.get_model();

  glUniform4f(athi_rect_manager.uniform[athi_rect_manager.COLOR], color.r, color.g, color.b, color.a);
  glUniformMatrix4fv(athi_rect_manager.uniform[athi_rect_manager.TRANSFORM], 1, GL_FALSE, &trans[0][0]);
  glDrawElements(draw_type, 6, GL_UNSIGNED_SHORT, NULL);
}

void draw_rects()
{
  athi_rect_manager.draw();
}


