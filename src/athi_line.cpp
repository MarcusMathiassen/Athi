#include "athi_line.h"
#include "athi_utility.h" // profile

vector<Athi_Line> line_immediate_buffer;
vector<Athi_Line *> line_buffer;
Athi_Line_Manager athi_line_manager;

void Athi_Line_Manager::init() {
  shader.init("Athi_Line_Manager::init()");
  shader.load_from_file("default_line_shader.vert", ShaderType::Vertex);
  shader.load_from_file("default_line_shader.geom", ShaderType::Geometry);
  shader.load_from_file("default_line_shader.frag", ShaderType::Fragment);
  shader.link();
  shader.add_uniform("positions");
  shader.add_uniform("color");

  gpu_buffer.init();
}

void Athi_Line_Manager::draw() {
  if (line_buffer.empty() && line_immediate_buffer.empty()) return;
  profile p("Athi_Line_Manager::draw()");
  gpu_buffer.bind();
  shader.bind();

  for (const auto &line : line_buffer) {
    shader.set_uniform("color", line->color);
    shader.set_uniform("positions",
                      vec4(line->p1.x, line->p1.y, line->p2.x, line->p2.y));
    glDrawArrays(GL_LINES, 0, 2);
  }

  for (const auto &line : line_immediate_buffer) {
    shader.set_uniform("color", line.color);
    shader.set_uniform("positions",
                      vec4(line.p1.x, line.p1.y, line.p2.x, line.p2.y));
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
