#include "athi_line.h"
#include "athi_rect.h"

void draw_line(const vec2& p1, const vec2& p2, f32 width, const vec4& color)
{
  glBindVertexArray(athi_rect_manager.VAO);
  glUseProgram(athi_rect_manager.shader_program);

  Transform temp{vec3(p1,0), vec3(), vec3(1,1,1)};
  //const f32 inverse_aspect = 1.0f / (f32)camera.aspect_ratio;

  // Rotate it 
  const f32 dx = p2.x - p1.x;
  const f32 dy = p2.y - p1.y;
  const f32 angle = -atan2(dy, dx);
  temp.rot.z = angle;

  temp.scale = vec3(width, dy, 0.0f);
  mat4 trans = temp.get_model(); 

  glUniform4f(athi_rect_manager.uniform[athi_rect_manager.COLOR], color.r, color.g, color.b, color.a);
  glUniformMatrix4fv(athi_rect_manager.uniform[athi_rect_manager.TRANSFORM], 1, GL_FALSE, &trans[0][0]);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}