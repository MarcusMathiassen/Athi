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

#include "athi_renderer.h" // circle_Renderer
#include "../athi_transform.h" // Transform
#include "athi_camera.h" // camera
#include "../Utility/athi_constant_globals.h" // kPI
#include "../Utility/threadsafe_container.h" // ThreadSafe::vector

struct circle {
  vec2 pos{0.0f, 0.0f};
  float radius{5.0f};
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
};

static ThreadSafe::vector<circle> circle_buffer;
static vector<mat4> models;
static vector<vec4> colors;

static Renderer circle_renderer;

static constexpr s32 circle_vertices = 36;

void init_circle_renderer()
{
  auto &shader = circle_renderer.make_shader();
  shader.sources = {"default_circle_shader.vert", "default_circle_shader.frag"};
  shader.attribs = {"position", "transform", "color"};

  // Setup the particle vertices
  vector<vec2> positions(circle_vertices);
  for (s32 i = 0; i < circle_vertices; ++i)
  {
    positions[i] = {cosf(i * kPI * 2.0f / circle_vertices),
                    sinf(i * kPI * 2.0f / circle_vertices)};
  }

  auto &vertex_buffer = circle_renderer.make_buffer("positions");
  vertex_buffer.data = &positions[0];
  vertex_buffer.data_size = circle_vertices * sizeof(positions[0]);
  vertex_buffer.data_members = 2;
  vertex_buffer.type = buffer_type::array_buffer;
  vertex_buffer.usage = buffer_usage::static_draw;

  auto &colors = circle_renderer.make_buffer("colors");
  colors.data_members = 4;
  colors.divisor = 1;

  auto &transforms = circle_renderer.make_buffer("transforms");
  transforms.data_members = 4;
  transforms.stride = sizeof(mat4);
  transforms.pointer = sizeof(vec4);
  transforms.divisor = 1;
  transforms.is_matrix = true;


  circle_renderer.finish();
}

void render_circles() noexcept
{
  if (circle_buffer.empty()) return;

  profile p("render_circles");

  if (models.size() < circle_buffer.size()) {
    models.resize(circle_buffer.size());
    colors.resize(circle_buffer.size());
  }

  const auto proj = camera.get_ortho_projection();
  {
    circle_buffer.lock();
    profile p("render_circles::update_buffers with new data");
    for (u32 i = 0; i < circle_buffer.size(); ++i)
    {
      auto &circle = circle_buffer[i];

      colors[i] = circle.color;
      Transform temp;
      temp.pos = vec3(circle.pos, 0.0f);
      temp.scale = vec3(circle.radius, circle.radius, 0.0f);
      models[i] = proj * temp.get_model();
    }
    circle_buffer.unlock();
  }

  {
    profile p("render_circles::update_buffers");
    circle_renderer.update_buffer("transforms", &models[0], sizeof(mat4) * circle_buffer.size());
    circle_renderer.update_buffer("colors", &colors[0], sizeof(vec4) * circle_buffer.size());
  }

  CommandBuffer cmd;
  cmd.type = primitive::triangle_fan;
  cmd.count = circle_vertices;
  cmd.primitive_count = static_cast<s32>(circle_buffer.size());

  circle_renderer.bind();
  {
    profile p("render_circles::circle_renderer.draw(cmd)");
    circle_renderer.draw(cmd);
  }

  circle_buffer.clear();
}

void draw_circle(const vec2 &pos, float radius, const vec4 &color, bool is_hollow) noexcept
{
  circle c;
  c.pos = pos;
  c.radius = radius;
  c.color = color;

  // @Incomplete: is_hollow ignored for now. Not implemented.
  circle_buffer.emplace_back(c);
}
