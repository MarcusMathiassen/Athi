
#include "athi_rect.h"
#include "athi_camera.h"
#include "athi_renderer.h"
#include "athi_transform.h"
#include "athi_utility.h"
#include "athi_particle.h"

#include <iostream>

std::vector<Athi_Rect> rect_immediate_buffer;
std::vector<Athi_Rect*> rect_buffer;
Athi_Rect_Manager athi_rect_manager;

namespace Athi {
bool Rect::contains(u32 id) {
  auto circle_pos = particle_manager.particles[id].pos;
  auto circle_radius = particle_manager.particles[id].radius;

  const glm::vec2 o = circle_pos;
  const float r = circle_radius;
  if (o.x - r < max.x && o.x + r > min.x && o.y - r < max.y && o.y + r > min.y)
    return true;
  return false;
}

bool Rect::contain_rect(const Rect& r) const {
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
  shader_program = glCreateProgram();
  const u32 vs =
      createShader("../Resources/athi_rect_shader.vs", GL_VERTEX_SHADER);
  const u32 fs =
      createShader("../Resources/athi_rect_shader.fs", GL_FRAGMENT_SHADER);

  glAttachShader(shader_program, vs);
  glAttachShader(shader_program, fs);

  glLinkProgram(shader_program);
  glValidateProgram(shader_program);
  validateShaderProgram("rect_constructor", shader_program);

  glDetachShader(shader_program, vs);
  glDetachShader(shader_program, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glGenBuffers(NUM_BUFFERS, VBO);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[INDICES]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  uniform[TRANSFORM] = glGetUniformLocation(shader_program, "transform");
  uniform[COLOR] = glGetUniformLocation(shader_program, "color");
}

void Athi_Rect_Manager::draw() {
  glBindVertexArray(VAO);
  glUseProgram(shader_program);

  const auto proj = camera.get_ortho_projection();
  
  for (auto& rect : rect_buffer) {
    rect->transform.pos = vec3(rect->pos, 0.0f);
    rect->transform.scale = vec3(rect->width, rect->height, 0);

    mat4 trans = proj * rect->transform.get_model();

    glUniform4f(uniform[COLOR], rect->color.r, rect->color.g, rect->color.b,
                rect->color.a);
    glUniformMatrix4fv(uniform[TRANSFORM], 1, GL_FALSE, &trans[0][0]);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
  }

  for (auto& rect : rect_immediate_buffer) {
    rect.transform.pos = vec3(rect.pos, 0.0f);
    rect.transform.scale = vec3(rect.width, rect.height, 0);

    mat4 trans = proj * rect.transform.get_model();

    glUniform4f(uniform[COLOR], rect.color.r, rect.color.g, rect.color.b,
                rect.color.a);
    glUniformMatrix4fv(uniform[TRANSFORM], 1, GL_FALSE, &trans[0][0]);
    glDrawElements(rect.draw_mode, 6, GL_UNSIGNED_SHORT, NULL);
  }
  rect_immediate_buffer.clear();
}

void init_rect_manager() { athi_rect_manager.init(); }

void add_rect(Athi_Rect* rect) { rect_buffer.emplace_back(rect); }

void draw_rect(const vec2& min, const vec2& max, const vec4& color,
               GLenum draw_type) {
  Athi_Rect rect;
  rect.pos = min;
  rect.width = max.x - min.x;
  rect.height = max.y - min.y;
  rect.draw_mode = draw_type;
  rect.color = color;

  rect_immediate_buffer.emplace_back(rect);
}

void draw_rect(const vec2& min, f32 width, f32 height, const vec4& color,
               GLenum draw_type) {
  Athi_Rect rect;
  rect.pos = min;
  rect.width = width;
  rect.height = height;
  rect.draw_mode = draw_type;
  rect.color = color;

  rect_immediate_buffer.emplace_back(rect);
}

void draw_hollow_rect(const vec2& min, const vec2& max, const vec4& color) {
  
  render_call([min, max, color]() {
    glBindVertexArray(athi_rect_manager.VAO);
    glUseProgram(athi_rect_manager.shader_program);

    auto max_ = max;
    auto min_ = min;
    const auto proj = camera.get_ortho_projection();

    max_.x -= 0.7f;
    max_.y -= 0.7f;
    min_.x += 0.7f;
    min_.y += 0.7f;

    Transform temp{vec3(min_, 0), vec3(), vec3(1, 1, 1)};

    const f32 width = max_.x - min_.x;
    const f32 height = max_.y - min_.y;
    temp.scale = vec3(width, height, 0);
    mat4 trans = proj *  temp.get_model();

    glUniform4f(athi_rect_manager.uniform[athi_rect_manager.COLOR], color.r,
                color.g, color.b, color.a);
    glUniformMatrix4fv(athi_rect_manager.uniform[athi_rect_manager.TRANSFORM],
                       1, GL_FALSE, &trans[0][0]);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
  });
}

void draw_rects() { athi_rect_manager.draw(); }
