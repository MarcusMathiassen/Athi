#version 410

out vec4 fragColor;

uniform sampler2D tex;

in Vertex
{
  vec2 textCoord;
  vec4 color;
} frag;

void main()
{
  if (frag.color == vec4(0,0,0,1)) discard;
  fragColor = texture(tex, frag.textCoord) * frag.color;
}
