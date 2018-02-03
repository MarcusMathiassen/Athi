#include "athi_circle.h"

#include "athi_renderer.h" // Renderer
#include "athi_transform.h" // Transform
#include "athi_camera.h" // camera
#include "../Utility/athi_globals.h" // kPI

static Renderer circle_renderer;

static constexpr s32 circle_vertices = 36;  

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

void draw_hollow_circle(const vec2 &pos, float radius, const vec4 &color) {
  render_call([pos, radius, color]{
    profile p("draw_hollow_circle");

    CommandBuffer cmd;
    cmd.type = primitive::triangle_fan;
    cmd.count = circle_vertices;
    circle_renderer.bind();

    const auto proj = camera.get_ortho_projection();

    Transform temp;
    temp.pos = {pos.x, pos.y, 0.0f};
    mat4 model = proj * temp.get_model();

    circle_renderer.shader.set_uniform("color", color);
    circle_renderer.shader.set_uniform("transform", model);
    circle_renderer.draw(cmd);
  });
}