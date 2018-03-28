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

#include "../athi_typedefs.h"

struct Rect {
  vec2 min{0.0f, 0.0f}, max{0.0f, 0.0f};
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Rect() = default;
  Rect(const vec2 &min_pos, const vec2 &max_pos) noexcept
      : min(min_pos), max(max_pos) {}
  constexpr bool contains(const vec2 &pos, f32 radius) const noexcept {
    if (pos.x - radius < max.x && pos.x + radius > min.x &&
        pos.y - radius < max.y && pos.y + radius > min.y)
      return true;
    return false;
  }
};

struct Athi_Rect {
  vec2 pos;
  vec2 min, max;
  float width, height;
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
};

void init_rect_renderer() noexcept;
void draw_rounded_rect(const vec2 &min, f32 width, f32 height, const vec4 &color, bool is_hollow) noexcept;
void draw_rounded_rect(const vec2 &min, const vec2 &max, const vec4 &color, bool is_hollow = false) noexcept;
void draw_rect(const vec2 &min, const vec2 &max, const vec4 &color, bool is_hollow = false) noexcept;
void draw_rect(const vec2 &min, f32 width, f32 height, const vec4 &color, bool is_hollow = false) noexcept;
void render_rects() noexcept;

void immidiate_draw_rounded_rect(const vec2 &min, const vec2 &max, const vec4 &color, bool is_hollow = false) noexcept;
void immidiate_draw_rounded_rect(const vec2 &min, f32 width, f32 height, const vec4 &color, bool is_hollow = false) noexcept;
