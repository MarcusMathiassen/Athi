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


#include "athi_rect.h"

#include "athi_camera.h"    // camera
#include "athi_renderer.h"  // render_call
#include "athi_line.h"   // draw_line
#include "athi_circle.h"   // draw_circle

#include "../athi_utility.h"   // profile
#include "../athi_transform.h" // Transform
#include "../Utility/threadsafe_container.h" // ThreadSafe::vector

static ThreadSafe::vector<Athi_Rect> rect_buffer;

static Renderer renderer;

static vector<mat4> models;
static vector<vec4> colors;

void init_rect_renderer() noexcept
{
  auto &shader = renderer.make_shader();
  shader.sources = {"default_rect_shader.vert", "default_rect_shader.frag"};
  shader.attribs = {"color", "transform"};

  auto &colors = renderer.make_buffer("colors");
  colors.data_members = 4;
  colors.divisor = 1;

  auto &transforms = renderer.make_buffer("transforms");
  transforms.data_members = 4;
  transforms.stride = sizeof(mat4);
  transforms.pointer = sizeof(vec4);
  transforms.divisor = 1;
  transforms.is_matrix = true;

  renderer.finish();
}

void render_rects() noexcept
{
  if (rect_buffer.empty()) return;

  if (models.size() < rect_buffer.size()) {
    models.resize(rect_buffer.size());
    colors.resize(rect_buffer.size());
  }

  const auto proj = camera.get_ortho_projection();
  {
    rect_buffer.lock();
    for (u32 i = 0; i < rect_buffer.size(); ++i)
    {
      auto &rect = rect_buffer[i];

      colors[i] = rect.color;

      Transform temp;
      temp.pos = vec3(rect.min, 0.0f);
      temp.scale = vec3(rect.width, rect.height, 1);

      models[i] = proj * temp.get_model();
    }
    rect_buffer.unlock();
  }

  {
    renderer.update_buffer("transforms", models);
    renderer.update_buffer("colors", colors);
  }

  {
    CommandBuffer cmd;
    cmd.type = primitive::triangles;
    cmd.count = 6;
    cmd.primitive_count = static_cast<s32>(rect_buffer.size());

    renderer.bind();
    renderer.draw(cmd);
  }

  rect_buffer.clear();
}


void draw_rounded_rect(const vec2 &min, f32 width, f32 height, const vec4 &color, bool is_hollow) noexcept
{
  draw_rounded_rect(min, vec2{min.x+width, min.y+height}, color, is_hollow);
}

void draw_rounded_rect(const vec2 &min, const vec2 &max, const vec4 &color, bool is_hollow) noexcept
{

  //  o---o
  //  |   |
  //  |   |
  //  o---o

  // Circles
  const float height = max.y - min.y;

  const float circle_radius = height * 0.25f;

  draw_circle(vec2(min.x, max.y),  circle_radius, color, is_hollow); // Left top
  draw_circle(min,                 circle_radius, color, is_hollow); // Left bottom
  draw_circle(vec2(max.x, min.y),  circle_radius, color, is_hollow); // Right bottom
  draw_circle(max,                 circle_radius, color, is_hollow); // Right top

  // Rects
  draw_rect(vec2(min.x-circle_radius, min.y), vec2(max.x+circle_radius, max.y),  color, is_hollow);
  draw_rect(vec2(min.x, min.y-circle_radius), vec2(max.x, max.y+circle_radius),  color, is_hollow);
}


void draw_rect(const vec2 &min, f32 width, f32 height, const vec4 &color, bool is_hollow) noexcept
{
  draw_rect(min, vec2(min.x+width, min.y+height), color, is_hollow);
}

void draw_rect(const vec2 &min, const vec2 &max, const vec4 &color, bool is_hollow) noexcept
{
  // If we're drawing a hollow rectangle..
  if (is_hollow) {

    // Draw using lines
    draw_line(min, vec2(min.x, max.y), 1.0f, color);
    draw_line(vec2(min.x, max.y), max, 1.0f, color);
    draw_line(max, vec2(max.x, min.y), 1.0f, color);
    draw_line(vec2(max.x, min.y), min, 1.0f, color);

    return;
  }

  Athi_Rect rect;
  rect.color = color;
  rect.min = min;
  rect.max = max;
  rect.width = max.x - min.x;
  rect.height = max.y - min.y;

  rect_buffer.emplace_back(rect);
}

void immidiate_draw_rounded_rect(const vec2 &min, const vec2 &max, const vec4 &color, bool is_hollow) noexcept
{

  //  o---o
  //  |   |
  //  |   |
  //  o---o

  // Circles
  const float height = max.y - min.y;

  const float circle_radius = height * 0.25f;

  // Corners
  draw_circle(vec2(min.x, max.y),  circle_radius, color, is_hollow); // Left top
  draw_circle(min,                 circle_radius, color, is_hollow); // Left bottom
  draw_circle(vec2(max.x, min.y),  circle_radius, color, is_hollow); // Right bottom
  draw_circle(max,                 circle_radius, color, is_hollow); // Right top

  // Body
  draw_rect(vec2(min.x-circle_radius, min.y), vec2(max.x+circle_radius, max.y),  color, is_hollow);
  draw_rect(vec2(min.x, min.y-circle_radius), vec2(max.x, max.y+circle_radius),  color, is_hollow);
}

// Wrapper
void immidiate_draw_rounded_rect(const vec2 &min, f32 width, f32 height, const vec4 &color, bool is_hollow) noexcept
{
  immidiate_draw_rounded_rect(min, {min.x+width, min.y+height}, color, is_hollow);
}
