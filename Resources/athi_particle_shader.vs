#version 330

in vec2 position;
in vec4 color;
in mat4 transform;

out Vertex { vec4 color; }
vertex;

void main() {
  gl_Position = transform * vec4(position, 0.0, 1.0);
  vertex.color = color;
}
