in vec4 position;
in vec4 color;

uniform mat4 ortho_projection;

out Vertex {
  vec2 texcoord;
  vec4 color;
} vertex;

void main()
{
    gl_Position = ortho_projection * vec4(position.xy, 0.0, 1.0);
    vertex.texcoord = position.zw;
    vertex.color = color;
}
