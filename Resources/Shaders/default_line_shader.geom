#version 410
layout (points) in;
layout (line_strip, max_vertices = 2) out;

uniform vec4 positions;

void main()
{
  gl_Position = gl_in[0].gl_Position + vec4(positions.xy, 0,0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(positions.zw, 0,0);
  EmitVertex();

  EndPrimitive();
}
