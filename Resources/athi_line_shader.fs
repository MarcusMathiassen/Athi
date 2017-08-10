#version 410

out vec4 frag_color;

in Geometry
{
  vec4 color;
} frag;

void main()
{
  frag_color = frag.color;
}
