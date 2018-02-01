#include "athi_rect.h"

#include "./Renderer/athi_camera.h"    // camera
#include "athi_particle.h"  // particle_system
#include "./Renderer/athi_renderer.h"  // render_call
#include "athi_utility.h"   // profile

std::vector<Athi_Rect> rect_immediate_buffer;
std::vector<Athi_Rect *> rect_buffer;
Athi_Rect_Manager athi_rect_manager;

namespace Athi {
bool Rect::contains(std::uint32_t id) {
  const auto o = particle_system.particles[id].pos;
  const auto r = particle_system.particles[id].radius;
  if (o.x - r < max.x && o.x + r > min.x && o.y - r < max.y && o.y + r > min.y)
    return true;
  return false;
}

bool Rect::contain_rect(const Rect &r) const {
  auto rmin = r.min;
  auto rmax = r.max;
  //  basic square collision check
  if (rmax.x < max.x && rmin.x > min.x && rmax.y < max.y && rmin.y > min.y) {
    return true;
  }
  return false;
}
}  // namespace Athi

void Athi_Rect_Manager::init() {

  auto &shader = renderer.make_shader();
  shader.sources = {"default_rect_shader.vert", "default_rect_shader.frag"};
  shader.uniforms = {"color", "transform"};

  u16 indices[]{0, 1, 2, 0, 2, 3};
  auto &indices_buffer = renderer.make_buffer("indices");
  indices_buffer.data = indices;
  indices_buffer.data_size = sizeof(indices);
  indices_buffer.type = buffer_type::element_array;

  renderer.finish();
}

void Athi_Rect_Manager::draw() {
  if (rect_buffer.empty() && rect_immediate_buffer.empty()) return;
  profile p("draw_rects");

  CommandBuffer cmd;
  cmd.type = primitive::triangles;
  cmd.count = 6;
  cmd.has_indices = true;
  renderer.bind();

  const auto proj = camera.get_ortho_projection();

  for (auto &rect : rect_buffer) {
    rect->transform.pos = vec3(rect->pos, 0.0f);
    rect->transform.scale = vec3(rect->width, rect->height, 0);

    mat4 trans = proj * rect->transform.get_model();
    renderer.shader.set_uniform("color", rect->color);
    renderer.shader.set_uniform("transform", trans);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    renderer.draw(cmd);
  }

  for (auto &rect : rect_immediate_buffer) {
    rect.transform.pos = vec3(rect.pos, 0.0f);
    rect.transform.scale = vec3(rect.width, rect.height, 0);

    mat4 trans = proj * rect.transform.get_model();
    renderer.shader.set_uniform("color", rect.color);
    renderer.shader.set_uniform("transform", trans);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    renderer.draw(cmd);
  }
  rect_immediate_buffer.clear();
}

void init_rect_manager() { athi_rect_manager.init(); }

void add_rect(Athi_Rect *rect) { rect_buffer.emplace_back(rect); }

void draw_rect(const vec2 &min, const vec2 &max, const vec4 &color,
               GLenum draw_type) {
  Athi_Rect rect;
  rect.pos = min;
  rect.width = max.x - min.x;
  rect.height = max.y - min.y;
  //rect.draw_mode = draw_type;
  rect.color = color;

  rect_immediate_buffer.emplace_back(rect);
}

void draw_rect(const vec2 &min, float width, float height, const vec4 &color,
               GLenum draw_type) {
  Athi_Rect rect;
  rect.pos = min;
  rect.width = width;
  rect.height = height;
  //rect.draw_mode = draw_type;
  rect.color = color;

  rect_immediate_buffer.emplace_back(rect);
}

void draw_hollow_rect(const vec2 &min, const vec2 &max, const vec4 &color) {

  render_call([min, max, color]{
    profile p("draw_hollow_rect");

    CommandBuffer cmd;
    cmd.type = primitive::line_loop;
    cmd.count = 4;
    athi_rect_manager.renderer.bind();

    auto max_ = max;
    auto min_ = min;
    const auto proj = camera.get_ortho_projection();

    max_.x -= 0.7f;
    max_.y -= 0.7f;
    min_.x += 0.7f;
    min_.y += 0.7f;

    Transform temp{vec3(min_, 0), vec3(), vec3(1, 1, 1)};

    const float width = max_.x - min_.x;
    const float height = max_.y - min_.y;
    temp.scale = vec3(width, height, 0);
    mat4 trans = proj * temp.get_model();

    athi_rect_manager.renderer.shader.set_uniform("color", color);
    athi_rect_manager.renderer.shader.set_uniform("transform", trans);
    athi_rect_manager.renderer.draw(cmd);
  });
}

void draw_rects() { athi_rect_manager.draw(); }
