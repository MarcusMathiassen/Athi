#version 330

uniform mat4 transform;
uniform vec4 color;

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
