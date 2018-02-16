in vec2 position;
in mat4 transform;
in vec4 color;

out Vertex {
  vec4 color;
} vertex;

void main() {
  gl_Position  = transform * vec4(position, 0.0, 1.0);
  vertex.color = color;
}
