#version 410

out vec4 frag_color;

in Vertex {
  vec4 color;
  vec2 texcoord;
} frag;

uniform sampler2D tex;

vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0.0);
  vec2 off1 = vec2(1.411764705882353) * direction;
  vec2 off2 = vec2(3.2941176470588234) * direction;
  vec2 off3 = vec2(5.176470588235294) * direction;
  color += texture(image, uv) * 0.1964825501511404;
  color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
  color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
  color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
  color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
  color +=
      texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
  color +=
      texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
  return color;
}

vec4 box_blur(sampler2D image, vec2 uv) {
  vec4 sum = vec4(0.0);
  for (int x = -4; x <= 4; x++)
    for (int y = -4; y <= 4; y++)
      sum += texture(image, vec2(uv.x + x * 1.0 / 200.0,
                                 uv.y + y * 1.0 / 200.0)) /
             81.0;
  return sum;
}

void main() {
  //vec4 hor = blur13(tex, frag.texcoord, vec2(1280 * 2, 800 * 2), vec2(1, 0));
  //vec4 vert = blur13(tex, frag.texcoord, vec2(1280 * 2, 800 * 2), vec2(0, 1));

  //vec4 gaussian = mix(hor, vert, 0.5);
  vec4 box_blur = box_blur(tex, frag.texcoord);
  //vec4 image = texture(tex, frag.texcoord);

  frag_color = box_blur;
}
