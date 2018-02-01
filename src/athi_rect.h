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


#pragma once

#include "athi_typedefs.h"  

#include "./Renderer/athi_renderer.h"  // Renderer
#include "athi_transform.h" // Transform

struct Rect {
  vec2 min_pos{0.0f, 0.0f}, max_pos{0.0f, 0.0f};
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Rect() = default;
  Rect(const vec2 &min, const vec2 &max) noexcept
      : min_pos(min), max_pos(max) {}
  constexpr bool contains(const vec2 &pos, f32 radius) const noexcept {
    if (pos.x - radius < max_pos.x && pos.x + radius > min_pos.x &&
        pos.y - radius < max_pos.y && pos.y + radius > min_pos.y)
      return true;
    return false;
  }
};

namespace Athi {
struct Rect {
  vec2 min, max;
  vec2 pos;
  f32 width, height;
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Rect(const vec2 &_min, const vec2 &_max) {
    min = _min;
    max = _max;
    pos = _min;
  }
  bool contain_rect(const Rect &r) const;
  bool contains(u32 id);
  Rect() = default;
};
}  // namespace Athi
struct Athi_Rect {
  Transform transform;
  vec2 pos{0, 0};
  f32 width;
  f32 height;
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

  Athi_Rect() = default;
  void draw() const {}
};

struct Athi_Rect_Manager {
  Renderer renderer;
  void init();
  void update();
  void draw();
};

void add_rect(Athi_Rect *rect);
void init_rect_manager();
void draw_rect(const vec2 &min, f32 width, f32 height, const vec4 &color,
               GLenum draw_type);
void draw_rect(const vec2 &min, const vec2 &max, const vec4 &color,
               GLenum draw_type);
void draw_hollow_rect(const vec2 &min, const vec2 &max, const vec4 &color);
void draw_rects();

void draw_line(const vec2 &p1, const vec2 &p2, f32 width, const vec4 &color);

extern std::vector<Athi_Rect> rect_immediate_buffer;
extern std::vector<Athi_Rect *> rect_buffer;
extern Athi_Rect_Manager athi_rect_manager;
