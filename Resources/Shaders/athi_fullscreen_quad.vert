#version 330

in vec2 position;
in vec4 color;
in vec2 texcoord;

uniform vec2 res;
uniform mat4 transform;
uniform vec2 dir;

const vec2 positions[4] = vec2[4](
    vec2(0.0f, 1.0f),
    vec2(1.0f, 1.0f),
    vec2(1.0f, 0.0f),
    vec2(0.0f, 0.0f));

out Vertex {
  vec4 color;
  vec2 texcoord;
} vertex;

void main() {
    vec2 pos = positions[gl_VertexID];
    pos.x  = pos.x * res.x;
    pos.y  = pos.y * res.y;
   gl_Position = transform * vec4(pos, 0.0, 1.0);
   vertex.color = color;
   vertex.texcoord = texcoord;
}
