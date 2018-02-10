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
#include "../athi_utility.h" // profile

#include "athi_renderer.h"      // Shader

static std::vector<Athi_Line> line_buffer;

static Renderer renderer;

static vector<vec4> positions;
static vector<vec4> colors;

void init_line_renderer() noexcept
{
  auto &shader = renderer.make_shader();
  shader.sources =  {
    "default_line_shader.vert", 
    "default_line_shader.geom", 
    "default_line_shader.frag",
  };

  shader.attribs = { 
    "positions", 
    "color",
  };

  auto &positions_buffer = renderer.make_buffer("positions");
  positions_buffer.data_members = 4;
  positions_buffer.divisor = 1;

  auto &colors_buffer = renderer.make_buffer("color");
  colors_buffer.data_members = 4;
  colors_buffer.divisor = 1;


  auto &vertex_buffer = renderer.make_buffer("empty");

  renderer.finish();
}

void render_lines() noexcept
{
  if (line_buffer.empty()) return;

  profile p("render_lines");

  if (positions.size() < line_buffer.size()) {
    positions.resize(line_buffer.size());
    colors.resize(line_buffer.size());
  }

  {
    profile p("render_lines::update_buffers with new data"); 
    for (u32 i = 0; i < line_buffer.size(); ++i)
    {
      auto &p1 = line_buffer[i].p1;
      auto &p2 = line_buffer[i].p2;
      positions[i] = vec4(p1.x, p1.y, p2.x, p2.y);
      colors[i] = line_buffer[i].color;
    }
  }

  {
    profile p("render_lines::update_buffers"); 
    // Update the gpu buffers incase of more particles..
    renderer.update_buffer("positions", &positions[0], sizeof(vec4) * line_buffer.size());
    renderer.update_buffer("color", &colors[0], sizeof(vec4) * line_buffer.size());
  }

  CommandBuffer cmd;
  cmd.type = primitive::points;
  cmd.count = 1;
  cmd.primitive_count = line_buffer.size();

  renderer.bind();
  {
    profile p("render_lines::renderer.draw(cmd)");
    renderer.draw(cmd);
  }

  line_buffer.clear();
}

void draw_line(const vec2 &p1, const vec2 &p2, f32 width, const vec4 &color) noexcept
{
  Athi_Line line;
  line.p1 = to_view_space(p1);
  line.p2 = to_view_space(p2);
  line.color = color;

  line_buffer.emplace_back(line);
}