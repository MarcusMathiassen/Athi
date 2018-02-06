out vec4 frag_color;

in Vertex {
  vec2 texcoord;
  vec4 color;
  vec2 pos;
  float radius;
} frag;

uniform sampler2D tex;

void main() {  
  vec4 image = texture(tex, frag.texcoord);
  if (image.r < 0.15) discard;
  frag_color = image * frag.color;
}