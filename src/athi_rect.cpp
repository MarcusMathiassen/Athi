
#include "athi_rect.h"
#include "athi_camera.h"
#include "athi_particle.h"
#include "athi_renderer.h"
#include "athi_transform.h"
#include "athi_utility.h"

#include <iostream>

std::vector<Athi_Rect> rect_immediate_buffer;
std::vector<Athi_Rect *> rect_buffer;
Athi_Rect_Manager athi_rect_manager;

namespace Athi {
bool Rect::contains(std::uint32_t id) {
  const auto o = particle_manager.particles[id].pos;
  const auto r = particle_manager.particles[id].radius;
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

Athi_Rect_Manager::~Athi_Rect_Manager() {
  glDeleteBuffers(NUM_BUFFERS, VBO);
  glDeleteVertexArrays(1, &VAO);
}

void Athi_Rect_Manager::init() {
  shader.init("Athi_Rect_Manager::init()");
  shader.load_from_file("../Resources/default_rect_shader.vert",
                        ShaderType::Vertex);
  shader.load_from_file("../Resources/default_rect_shader.frag",
                        ShaderType::Fragment);
  shader.link();
  shader.add_uniform("color");
  shader.add_uniform("transform");

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(NUM_BUFFERS, VBO);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[INDICES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
}

void Athi_Rect_Manager::draw() {
  if (rect_buffer.empty() && rect_immediate_buffer.empty()) return;
  profile p("draw_rects");

  glBindVertexArray(VAO);
  shader.bind();

  const auto proj = camera.get_ortho_projection();

  for (auto &rect : rect_buffer) {
    rect->transform.pos = vec3(rect->pos, 0.0f);
    rect->transform.scale = vec3(rect->width, rect->height, 0);

    mat4 trans = proj * rect->transform.get_model();
    shader.setUniform("color", rect->color);
    shader.setUniform("transform", trans);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
  }

  for (auto &rect : rect_immediate_buffer) {
    rect.transform.pos = vec3(rect.pos, 0.0f);
    rect.transform.scale = vec3(rect.width, rect.height, 0);

    mat4 trans = proj * rect.transform.get_model();
    shader.setUniform("color", rect.color);
    shader.setUniform("transform", trans);
    glDrawElements(rect.draw_mode, 6, GL_UNSIGNED_SHORT, NULL);
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
  rect.draw_mode = draw_type;
  rect.color = color;

  rect_immediate_buffer.emplace_back(rect);
}

void draw_rect(const vec2 &min, float width, float height, const vec4 &color,
               GLenum draw_type) {
  Athi_Rect rect;
  rect.pos = min;
  rect.width = width;
  rect.height = height;
  rect.draw_mode = draw_type;
  rect.color = color;

  rect_immediate_buffer.emplace_back(rect);
}

void draw_hollow_rect(const vec2 &min, const vec2 &max, const vec4 &color) {
  render_call([min, max, color]() {
    glBindVertexArray(athi_rect_manager.VAO);
    athi_rect_manager.shader.bind();

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

    athi_rect_manager.shader.setUniform("color", color);
    athi_rect_manager.shader.setUniform("transform", trans);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
  });
}

void draw_rects() { athi_rect_manager.draw(); }
