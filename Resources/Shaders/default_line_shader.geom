#version 410
layout (points) in;
layout (line_strip, max_vertices = 2) out;


in Vertex {
  vec4 positions;
  vec4 color;
} vertex[];

out Geometry
{
  vec4 color;
} geometry;

void main()
{
  geometry.color = vertex[0].color;

  gl_Position = gl_in[0].gl_Position + vec4(vertex[0].positions.xy, 0,0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(vertex[0].positions.zw, 0,0);
  EmitVertex();

  EndPrimitive();
}
