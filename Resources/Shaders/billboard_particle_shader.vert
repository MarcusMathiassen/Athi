#version 410

in float radius;
in vec4 color;
in mat4 transform;

const vec2 texcoords[4] = vec2[4](
  vec2(0.0f, 1.0f), 
  vec2(1.0f, 1.0f), 
  vec2(1.0f, 0.0f), 
  vec2(0.0f, 0.0f));

const vec2 positions[4] = vec2[4](
    vec2(0.0f, 1.0f),
    vec2(1.0f, 1.0f),
    vec2(1.0f, 0.0f),
    vec2(0.0f, 0.0f));

out Vertex {
  vec2 texcoord;
  vec4 color;
  vec2 pos;
  float radius;
} vertex;

#define SIZE 2.7
#define POS_OFFSET 0.25

void main() {
  gl_Position = transform * vec4(positions[gl_VertexID]*SIZE - radius*POS_OFFSET, 0.0, 1.0);
  vertex.texcoord = texcoords[gl_VertexID];
  vertex.color = color;
  vertex.pos = vec2(transform[3].xy);
  vertex.radius = radius;
}

