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

#include "./Renderer/athi_camera.h"    // camera
#include "./Renderer/athi_renderer.h"  // render_call

#include "athi_utility.h"   // profile
#include "athi_transform.h" // Transform

static std::vector<Athi_Rect> rect_buffer;

static Renderer renderer;

static vector<mat4> models;
static vector<vec4> colors;

void draw_rects() noexcept
{
  profile p("draw_rects");


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
    profile p("draw_rects::update_buffers"); 
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
}


void draw_rect(const vec2 &min, const vec2 &max, const vec4 &color, bool is_hollow) noexcept
{
  // If we're drawing a hollow rectangle, dont draw it with indices
  if (is_hollow) {
    // Draw using lines instead.
  }

  Athi_Rect rect;
  rect.color = color;
  rect.min = min;
  rect.max = max;
  rect.width = max.x - min.x;
  rect.height = max.y - min.y;

  rect_buffer.emplace_back(rect);
}
