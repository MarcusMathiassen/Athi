#version 410

in vec3 position;
in vec4 color;
in mat4 transform;

out vec4 color0;

void main()
{
  gl_Position = transform * vec4(position, 1.0);
  color0 = color;
}
