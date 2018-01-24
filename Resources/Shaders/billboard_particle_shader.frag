#version 330

out vec4 frag_color;

in Vertex { 
  vec2 texcoord;
  vec4 color;
} frag;

uniform sampler2D sprite_atlas;

void main() {

  frag_color = texture(sprite_atlas, frag.texcoord) * frag.color;
}
