out vec4 frag_color;

uniform vec2 dir;
uniform vec2 res;

uniform sampler2D tex;

in Vertex {
  vec2 texcoord;
}
frag;

void main() {
  frag_color = blur13(tex, frag.texcoord, vec2(res.x, res.y), dir);
}