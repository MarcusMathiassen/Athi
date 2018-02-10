// Copyright (c) 2018 Marcus Mathiassen

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.


#include "athi_circle.h"

#include "athi_renderer.h" // Renderer
#include "athi_transform.h" // Transform
#include "athi_camera.h" // camera
#include "../Utility/athi_constant_globals.h" // kPI

static Renderer circle_renderer;

static constexpr s32 circle_vertices = 360;

void init_circle_renderer() {

  auto &shader = circle_renderer.make_shader();
  shader.sources = {"default_circle_shader.vert", "default_circle_shader.frag"};
  shader.uniforms = {"color", "transform"};

  // Setup the particle vertices
  vector<vec2> positions(circle_vertices);
  for (s32 i = 0; i < circle_vertices; ++i)
  {
    positions[i] = {cosf(i * kPI * 2.0f / circle_vertices),
                    sinf(i * kPI * 2.0f / circle_vertices)};
  }

  auto &vertex_buffer = circle_renderer.make_buffer("position");
  vertex_buffer.data = &positions[0];
  vertex_buffer.data_size = circle_vertices * sizeof(positions[0]);
  vertex_buffer.data_members = 2;
  vertex_buffer.type = buffer_type::array;
  vertex_buffer.usage = buffer_usage::static_draw;

  circle_renderer.finish();
}
    
void draw_circle(const vec2 &pos, float radius, const vec4 &color, primitive prim_type = primitive::triangle_fan)
{
    CommandBuffer cmd;
    cmd.type = prim_type;
    cmd.count = circle_vertices;
}

void draw_filled_circle(const vec2 &pos, float radius, const vec4 &color) noexcept {
  render_call([pos, radius, color]{

    CommandBuffer cmd;
    cmd.type = primitive::triangle_fan;
    cmd.count = circle_vertices;
    circle_renderer.bind();

    const auto proj = camera.get_ortho_projection();

    Transform temp;
    temp.pos = {pos.x, pos.y, 0.0f};
    temp.scale = {radius, radius, 0.0f};
    mat4 model = proj * temp.get_model();

    circle_renderer.shader.set_uniform("color", color);
    circle_renderer.shader.set_uniform("transform", model);
    circle_renderer.draw(cmd);

  });
}

void draw_hollow_circle(const vec2 &pos, float radius, const vec4 &color) noexcept
{
    render_call([pos, radius, color]
    {
        CommandBuffer cmd;
        cmd.type = primitive::line_loop;
        cmd.count = circle_vertices;
        circle_renderer.bind();

        const auto proj = camera.get_ortho_projection();

        Transform temp;
        temp.pos = {pos.x, pos.y, 0.0f};
        temp.scale = {radius, radius, 0.0f};
        mat4 model = proj * temp.get_model();

        circle_renderer.shader.set_uniform("color", color);
        circle_renderer.shader.set_uniform("transform", model);
        circle_renderer.draw(cmd);
    });
}