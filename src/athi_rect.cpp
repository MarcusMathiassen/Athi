#include "athi_rect.h"

#include "athi_circle.h"
#include "athi_camera.h"
#include "athi_transform.h"

#include "athi_utility.h"


Athi_Rect::Athi_Rect(const vec2& min, const vec2& max) : min(min), max(max) {}

Athi_Rect::~Athi_Rect()
{
  glDeleteBuffers(NUM_BUFFERS, VBO);
  glDeleteVertexArrays(1, &VAO);
}

bool Athi_Rect::contains(u32 id) const
{
  const vec2 o  = athi_circle_manager.circle_buffer[id].pos;
  const f32 r   = athi_circle_manager.circle_buffer[id].radius;

  //  basic square collision check
  if (o.x - r < max.x && o.x + r > min.x && o.y - r < max.y &&
      o.y + r > min.y) {
    return true;
  }
  return false;
}

void Athi_Rect::init()
{
  shader_program  = glCreateProgram();
  const u32 vs    = createShader("../Resources/athi_rect_shader.vs", GL_VERTEX_SHADER);
  const u32 fs    = createShader("../Resources/athi_rect_shader.fs", GL_FRAGMENT_SHADER);

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

  uniform[TRANSFORM] = glGetUniformLocation(shader_program, "transform");
  uniform[COLOR]     = glGetUniformLocation(shader_program, "color");
}

// @Cleanup: make a parent class
void Athi_Rect::draw(GLenum draw_type) const
{
  const f32 inverse_aspect = 1.0f / (f32)camera.aspect_ratio;
  Transform temp{vec3(pos,0), vec3(), vec3(1,1,1)};
  temp.scale = vec3(inverse_aspect, 1, 0);
  mat4 trans = temp.get_model();

  glBindVertexArray(VAO);
  glUseProgram(shader_program);
  glUniformMatrix4fv(uniform[TRANSFORM], 1, GL_FALSE, &trans[0][0]);
  glUniform4f(uniform[COLOR], color.r, color.g, color.g, color.a);
  glDrawElements(draw_type, 6, GL_UNSIGNED_SHORT, NULL);
}
