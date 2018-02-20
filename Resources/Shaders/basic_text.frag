out vec4 frag_color;

uniform sampler2D tex;

in Vertex {
  vec2 texcoord;
  vec4 color;
} frag;

void main()
{
      vec4 sampled = vec4(1.0, 1.0, 1.0, texture(tex, frag.texcoord).r);
      frag_color = frag.color * sampled;
}
