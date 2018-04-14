in vec2     vertices;

in vec2     position;
in vec4     color;
in float    radius;

uniform vec2 viewport_size;

out Vertex { vec4 color; }
vertex;

void main()
{
    const vec2 fpos = (radius[gl_InstanceID] * vertices[gl_VertexID] + position[gl_InstanceID]) / (viewport_size / 2.0);

    gl_Position = vec4(fpos, 0, 1);
    vertex.color = color;
}
