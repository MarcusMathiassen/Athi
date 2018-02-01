#version 330

uniform vec4 color;
uniform mat4 transform;

const vec2 position[4] = vec2[4](
  vec2(0.0,  1.0),
  vec2(0.0,  0.0),
  vec2(1.0,  0.0),
  vec2(1.0,  1.0)
);

void main()
{
  gl_Position  = transform * vec4(position[gl_VertexID], 0.0, 1.0 );
}
