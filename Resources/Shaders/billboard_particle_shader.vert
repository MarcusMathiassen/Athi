#version 330

in vec2 position;
in vec4 color;
uniform vec2 resolution;
uniform vec2 position_offset;

out Vertex { vec4 color; }
vertex;

void main() {
  gl_Position = vec4(position+position_offset, 0.0, 1.0);
  vertex.color = color;
}
