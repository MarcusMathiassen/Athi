#version 330

out vec4 fragColor;

in Vertex
{
  vec4 color;
} frag;

void main()
{
  fragColor = frag.color;
}
