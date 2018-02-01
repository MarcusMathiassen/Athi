#include "athi_line.h"
#include "athi_utility.h" // profile

vector<Athi_Line> line_immediate_buffer;
vector<Athi_Line *> line_buffer;
Athi_Line_Manager athi_line_manager;

void Athi_Line_Manager::init() {

  auto &shader = renderer.make_shader();
  shader.sources =  {"default_line_shader.vert", "default_line_shader.geom", "default_line_shader.frag"};
  shader.uniforms = { "positions", "color"};

  auto &vertex_buffer = renderer.make_buffer("empty");

  renderer.finish();
}

void Athi_Line_Manager::draw() {
  if (line_buffer.empty() && line_immediate_buffer.empty()) return;
  profile p("Athi_Line_Manager::draw()");

  CommandBuffer l_cmd, r_cmd;
  l_cmd.type = primitive::lines;
  l_cmd.count = 2;
  r_cmd.type = primitive::points;
  r_cmd.count = 2;
  renderer.bind();

  for (const auto &line : line_buffer) {

    renderer.shader.set_uniform("color", line->color);
    renderer.shader.set_uniform("positions", vec4(line->p1.x, line->p1.y, line->p2.x, line->p2.y));
    glDrawArrays(GL_LINES, 0, 2);
    renderer.draw(l_cmd);
  }

  for (const auto &line : line_immediate_buffer) {
    renderer.shader.set_uniform("color", line.color);
    renderer.shader.set_uniform("positions", vec4(line.p1.x, line.p1.y, line.p2.x, line.p2.y));
    //glDrawArrays(GL_POINTS, 0, 2);
    renderer.draw(r_cmd);
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
