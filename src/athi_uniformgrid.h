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
#include "./Renderer/athi_rect.h"  // draw_rect, Rect

template <class T>
class UniformGrid {
 protected:
  static vector<T> data;

 private:
  struct Rect {
    vec2 min{0.0f, 0.0f}, max{0.0f, 0.0f};
    vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    Rect() = default;
    Rect(const vec2 &min_pos, const vec2 &max_pos) noexcept : min(min_pos), max(max_pos) {} 
    constexpr bool contains(const vec2 &pos, f32 radius) const noexcept { return (pos.x - radius < max.x && pos.x + radius > min.x && pos.y - radius < max.y && pos.y + radius > min.y); }
  };

  struct Node : public UniformGrid {
    Rect bounds;
    vector<s32> index;
    Node(const Rect &r) noexcept : bounds{r} {}
    void insert(s32 id) noexcept { index.emplace_back(id); }
    void get(vector<vector<s32>> &cont) const noexcept { cont.emplace_back(index); }
    void draw_bounds(const vec4& color) const noexcept { draw_rect(bounds.min, bounds.max, color, true); }
    bool contains(s32 id) const noexcept { return bounds.contains(this->data[id].pos, this->data[id].radius); }
  };


 public:

  vector<Node> nodes;

  void init(const vec2& min, const vec2& max, s32 parts) noexcept {
    nodes.clear();

    const f32 sqrtGrid = sqrt(parts);
    const f32 col = max.x / sqrtGrid;
    const f32 row = max.y / sqrtGrid;

    for (f32 y = min.y; y < max.y; y += row) {
      for (f32 x = min.x; x < max.x; x += col) {
        Rect bounds(vec2(x, y), vec2(x + col, y + row));
        nodes.emplace_back(bounds);
      }
    }
  };

  void set_data(const vector<T> &objects) noexcept {
    data = objects;
  }

  void multithreaded_input(size_t begin, size_t end) noexcept {
    for (size_t i = begin; i < end; ++i) {
      const auto id = this->data[i].id;
      for (auto &node : nodes) {
        if (node.contains(id)) {
          node.insert(id);
        }
      }
    }
  }

  void input(const vector<T> &objects) noexcept {
    data = objects;
    for (const auto &obj : data) {
      const auto id = obj.id;
      for (auto &node : nodes) {
        if (node.contains(id)) {
          node.insert(id);
        }
      }
    }
  }
  void draw_bounds(const vec4& color) const noexcept {
    for (const auto &node : nodes) {
      node.draw_bounds(color);
    }
  }

  void get_neighbours(vector<vector<s32>> &cont, const T& p) const noexcept {
    for (const auto &node : nodes) {
      if (node.bounds.contains(p.pos, p.radius)) {
        node.get(cont);
      }
    }
  }

  void get(vector<vector<s32>> &cont) const noexcept {
    for (const auto &node : nodes) {
      node.get(cont);
    }
  }
  void reset() noexcept {
    for (auto &node : nodes) {
      node.index.clear();
    }
  }
};

template <class T>
vector<T> UniformGrid<T>::data;