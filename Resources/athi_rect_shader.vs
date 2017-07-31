#version 410

in vec2 position;
uniform vec2 position_offset;
uniform vec4 color;

out Vertex
{
  vec4 color;
} vertex;

void main()
{
  vertex.color = color;
  gl_Position  = vec4(position_offset + position, 0.0, 1.0 );
}
