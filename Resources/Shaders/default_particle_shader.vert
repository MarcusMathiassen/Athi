in vec2     vertices;

in vec2     position;
in vec4     color;
in float    radius;

uniform vec2 viewport_size;

out Vertex { vec4 color; }
vertex;

void main()
{
    vec2 p = (radius * vertices + position);
    p.x = ((p.x-0.5) / (viewport_size.x*0.5))-1.0;
    p.y = ((p.y-0.5) / (viewport_size.y*0.5))-1.0;

    vec2 pos = p;

    gl_Position = vec4(pos, 0, 1);
    vertex.color = color;
}
