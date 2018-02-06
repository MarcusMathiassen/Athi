in vec4 positions;
in vec4 color;

out Vertex {
  vec4 positions;
  vec4 color;
} vertex;

void main()
{
  gl_Position = vec4(0,0,0,1);
  vertex.positions = positions;
  vertex.color = color;
}
