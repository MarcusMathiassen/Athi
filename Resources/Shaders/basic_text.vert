in vec4 position;
in vec2 texcoord;
in vec2 color;

uniform mat4 ortho_projection;

out Vertex {
  vec2 texcoord;
  vec4 color;
} vertex;

void main()
{
    gl_Position = ortho_projection * vec4(vertex, 0.0, 1.0);
    vertex.texcoord = texcoord;
    vertex.color = color;
};
