#version 410

in vec2 position;
in vec4 color;

out Vertex { 
  vec4 color; 
} vertex;

void main() {
  gl_PointSize = 1;
  gl_Position = vec4(position, 0.0, 1.0);
 
  vertex.color = color;
}
