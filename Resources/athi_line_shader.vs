#version 330

uniform vec4 positions;
uniform vec4 color;

out Vertex
{
  vec4 positions;
  vec4 color;
} vertex;

void main()
{
  gl_Position = vec4(0,0,0,1);
  vertex.positions = positions;
  vertex.color = color;
}
