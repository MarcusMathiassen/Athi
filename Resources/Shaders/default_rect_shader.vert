
#ifdef IMMIDIATE_MODE
uniform vec4 color;
uniform mat4 transform;
#else
in vec4 color;
in mat4 transform;
#endif

const vec2 texcoords[6] = vec2[6](
  vec2(0.0f, 1.0f),
  vec2(1.0f, 1.0f),
  vec2(1.0f, 0.0f),

  vec2(0.0f, 1.0f),
  vec2(1.0f, 0.0f),
  vec2(0.0f, 0.0f));

const vec2 positions[6] = vec2[6](
    vec2(-1.0f,  1.0f),
    vec2( 1.0f,  1.0f),
    vec2( 1.0f, -1.0f),

    vec2(-1.0f,  1.0f),
    vec2( 1.0f, -1.0f),
    vec2(-1.0f, -1.0f));

out Vertex {
  vec2 texcoord;
  vec4 color;
} vertex;

void main()
{
  gl_Position =  transform * vec4(positions[gl_VertexID], 0.0, 1.0);
  vertex.texcoord = texcoords[gl_VertexID];
  vertex.color = color;
}
