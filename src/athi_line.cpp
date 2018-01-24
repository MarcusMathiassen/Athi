#include "athi_line.h"
#include "athi_camera.h"
#include "athi_rect.h"
#include "athi_shader.h"

std::vector<Athi_Line> line_immediate_buffer;
std::vector<Athi_Line *> line_buffer;
Athi_Line_Manager athi_line_manager;

Athi_Line_Manager::~Athi_Line_Manager() { glDeleteVertexArrays(1, &VAO); }

void Athi_Line_Manager::init() {

  shader.init("Athi_Line_Manager::init()");
  shader.load_from_file("default_line_shader.vert", ShaderType::Vertex);
  shader.load_from_file("default_line_shader.geom", ShaderType::Geometry);
  shader.load_from_file("default_line_shader.frag", ShaderType::Fragment);
  shader.link();
  shader.add_uniform("positions");
  shader.add_uniform("color");

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
}

void Athi_Line_Manager::draw() {
  if (line_buffer.empty() && line_immediate_buffer.empty())
    return;
  profile p("Athi_Line_Manager::draw()");
  glBindVertexArray(VAO);
  shader.bind();

  for (const auto &line : line_buffer) {
    shader.setUniform("color", line->color);
    shader.setUniform("positions", glm::vec4(line->p1.x, line->p1.y, line->p2.x, line->p2.y));
    glDrawArrays(GL_LINES, 0, 2);
  }

  for (const auto &line : line_immediate_buffer) {

    shader.setUniform("color", line.color);
    shader.setUniform("positions", glm::vec4(line.p1.x, line.p1.y, line.p2.x, line.p2.y));
    glDrawArrays(GL_POINTS, 0, 2);
  }
  line_immediate_buffer.clear();
}

void init_line_manager() { athi_line_manager.init(); }

void add_line(Athi_Line *line) { line_buffer.emplace_back(line); }

void draw_lines() { athi_line_manager.draw(); }

void draw_line(const vec2 &p1, const vec2 &p2, f32 width, const vec4 &color) {
  Athi_Line line;
  line.p1 = p1;
  line.p2 = p2;
  line.width = width;
  line.color = color;

  line_immediate_buffer.emplace_back(line);
}
