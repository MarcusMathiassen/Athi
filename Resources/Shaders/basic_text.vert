in vec4 vertices;
uniform vec4 color;
uniform mat4 ortho_projection;

out Vertex {
  vec2 texcoord;
  vec4 color;
} vertex;

void main()
{
    gl_Position = ortho_projection * vec4(vertices.xy, 0.0, 1.0);
    vertex.texcoord = vertices.zw;
    vertex.color = color;
}
