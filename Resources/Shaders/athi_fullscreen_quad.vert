const vec2 texcoords[4] = vec2[4](
  vec2(0.0, 1.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0),
  vec2(0.0, 0.0)
);

const vec2 positions[4] = vec2[4](
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, -1.0)
);

out Vertex
{
  vec2 texcoord;
} vertex;

void main()
{
  vec2 pos = positions[gl_VertexID];
  gl_Position = vec4(pos, 0.0, 1.0);
  vertex.texcoord = texcoords[gl_VertexID];
}
