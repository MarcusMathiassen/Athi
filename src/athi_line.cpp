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
