#include "athi_line.h"
#include "athi_rect.h"

void draw_line(const vec2& p1, const vec2& p2, f32 width, const vec4& color)
{
  Transform temp{vec3(p1,0), vec3(), vec3(1,1,1)};

  // Rotate it
  const f32 dx = p2.x - p1.x;
  const f32 dy = p2.y - p1.y;
  const f32 angle = -atan2(dy, dx);
  temp.rot.z = angle;
}
