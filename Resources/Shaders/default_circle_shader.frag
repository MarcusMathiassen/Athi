out vec4 frag_color;

in Vertex {
  vec4 color;
} frag;

void main()
{
  frag_color = frag.color;
}
