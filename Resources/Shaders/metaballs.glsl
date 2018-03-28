#ifndef METABALLS_GLSL
#define METABALLS_GLSL

uniform vec2 tex_size;
uniform sampled2D tex;

void metaball()
{
    float sum = 0.0;

    int x;
    int y;

    // The balls are stored in the texture tex

    // Go through all balls..
    for (px = 0; px < tex_size.x; ++px)
    {
        for (py= 0; py < tex_size.y; ++py)
        {
            vec2 pixel = vec2(px,py);
            vec4 ball = texture(tex, pixel);
            sum += ball.z / distance(pixel, ball.xy);
        }
    }
    frag_color = vec4(1,1,1,sum);
}
#endif
