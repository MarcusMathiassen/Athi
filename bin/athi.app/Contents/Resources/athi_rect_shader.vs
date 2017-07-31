#version 410

in vec2 position;
uniform mat4 transform;
uniform vec4 color;

out Vertex
{
  vec4 color;
} vertex;

void main()
{
  vertex.color = color;
  gl_Position  = transform * vec4(position, 0.0, 1.0 );
}
