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

static std::vector<Athi_Rect> rect_buffer;

static Renderer renderer;
static Renderer immidiate_renderer;

static vector<mat4> models;
static vector<vec4> colors;

void render_rects() noexcept
{
  if (rect_buffer.empty()) return;

  profile p("render_rects");

  if (models.size() < rect_buffer.size()) {
    models.resize(rect_buffer.size());
    colors.resize(rect_buffer.size());
  }

  const auto proj = camera.get_ortho_projection();

  for (u32 i = 0; i < rect_buffer.size(); ++i)
  {
    auto &rect = rect_buffer[i];

    colors[i] = rect.color;

    Transform temp;
    temp.pos = vec3(rect.min, 0.0f);
    temp.scale = vec3(rect.width, rect.height, 1);

    models[i] = proj * temp.get_model();
  }

  {
    profile p("render_rects::update_buffers"); 
    // Update the gpu buffers incase of more particles..
    renderer.update_buffer("transforms", &models[0], sizeof(mat4) * rect_buffer.size());
    renderer.update_buffer("colors", &colors[0], sizeof(vec4) * rect_buffer.size());
  }

  CommandBuffer cmd;
  cmd.type = primitive::triangles;
  cmd.count = 6;
  cmd.has_indices = true;
  cmd.primitive_count = rect_buffer.size();

  renderer.bind();
  renderer.draw(cmd);

  rect_buffer.clear();
}

void init_rect_renderer() noexcept
{ 
  // Static renderer
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

  constexpr u16 indices[]{0, 1, 2, 0, 2, 3};
  auto &indices_buffer = renderer.make_buffer("indices");
  indices_buffer.data = (void*)indices;
  indices_buffer.data_size = sizeof(indices);
  indices_buffer.type = buffer_type::element_array;

  renderer.finish();

  // Immidiate renderer
  auto &immidiate_shader = immidiate_renderer.make_shader();
  immidiate_shader.sources = {"default_rect_shader.vert", "default_rect_shader.frag"};
  immidiate_shader.uniforms = {"color", "transform"};
  immidiate_shader.preambles = {"common.glsl"};

  auto &uindices_buffer = immidiate_renderer.make_buffer("indices");
  uindices_buffer.data = (void*)indices;
  uindices_buffer.data_size = sizeof(indices);
  uindices_buffer.type = buffer_type::element_array;

  immidiate_renderer.finish();
}


void draw_rounded_rect(const vec2 &min, f32 width, f32 height, const vec4 &color, bool is_hollow) noexcept
{
  const float circle_radius = height * 0.25f;

  const vec2 max{min.x+width, min.y+height};

  auto draw_circle_fp = is_hollow ? &draw_hollow_circle : &draw_filled_circle;

  draw_circle_fp(vec2(min.x, max.y),  circle_radius, color); // Left top
  draw_circle_fp(min,                 circle_radius, color); // Left bottom 
  draw_circle_fp(vec2(max.x, min.y),  circle_radius, color); // Right bottom
  draw_circle_fp(max,                 circle_radius, color); // Right top

  // Rects
  draw_rect(vec2(min.x-circle_radius, min.y), vec2(max.x+circle_radius, max.y),  color, is_hollow);
  draw_rect(vec2(min.x, min.y-circle_radius), vec2(max.x, max.y+circle_radius),  color, is_hollow);
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

  auto draw_circle_fp = is_hollow ? &draw_hollow_circle : &draw_filled_circle;

  draw_circle_fp(vec2(min.x, max.y),  circle_radius, color); // Left top
  draw_circle_fp(min,                 circle_radius, color); // Left bottom 
  draw_circle_fp(vec2(max.x, min.y),  circle_radius, color); // Right bottom
  draw_circle_fp(max,                 circle_radius, color); // Right top

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

    // immididate_draw_hollow_rect(min, max, color);

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


void immididate_draw_rect(const vec2 &min, const vec2 &max, const vec4 &color, bool is_hollow) noexcept
{
  render_call([min, max, color] {
    CommandBuffer cmd;
    cmd.type = primitive::triangles;
    cmd.count = 6;
    cmd.has_indices = true;
    immidiate_renderer.bind();

    const auto proj = camera.get_ortho_projection();

    Transform temp;
    temp.pos = vec3(min, 0);
    temp.scale = vec3(max.x - min.x, max.y - min.y, 0);
    mat4 trans = proj * temp.get_model();

    immidiate_renderer.shader.set_uniform("color", color);
    immidiate_renderer.shader.set_uniform("transform", trans);
    immidiate_renderer.draw(cmd);
  });
}

void immididate_draw_hollow_rect(const vec2 &min, const vec2 &max, const vec4 &color) noexcept
{
  render_call([min, max, color] {
    CommandBuffer cmd;
    cmd.type = primitive::line_loop;
    cmd.count = 4;
    immidiate_renderer.bind();

    const auto proj = camera.get_ortho_projection();

    Transform temp;
    temp.pos = vec3(min, 0);
    temp.scale = vec3(max.x - min.x, max.y - min.y, 0);
    mat4 trans = proj * temp.get_model();

    immidiate_renderer.shader.set_uniform("color", color);
    immidiate_renderer.shader.set_uniform("transform", trans);
    immidiate_renderer.draw(cmd);
  });
}