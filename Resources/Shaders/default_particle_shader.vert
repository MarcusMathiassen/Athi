in vec2     vertices;

in vec2     position;
in vec4     color;
in float    radius;

uniform vec2 viewport_size;

out Vertex { vec4 color; }
vertex;

void main()
{
    vec2 pos = ((radius * vertices + position) / viewport_size * 2.0) - 1.0;
    gl_Position = vec4(pos, 0, 1);
    vertex.color = color;
}
