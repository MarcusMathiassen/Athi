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

#include "athi_rect.h"  // draw_hollow_rect, Rect
#include "athi_settings.h" // universal_color_picker
#include "athi_utility.h" // get_universal_current_color

template <class T>
class UniformGrid {
 protected:
  static vector<T> data;

 private:
  struct Node : public UniformGrid {
    Rect bounds;
    vector<s32> index;
    Node(const Rect &r) : bounds{r} {}
    void insert(s32 id) { index.emplace_back(id); }
    void get(vector<vector<s32>> &cont) { cont.emplace_back(index); }
    void draw_bounds() const {
      draw_hollow_rect(bounds.min_pos, bounds.max_pos, bounds.color);
    }
    void color_objects(vector<vec4> &color) const {
      for (const auto i : index) {
        color[i] = bounds.color;
      }
    }

    bool contains(s32 id) {
      return bounds.contains(this->data[id].pos, this->data[id].radius);
    }
  };

  vector<std::unique_ptr<Node>> nodes;
  static s32 current_uniformgrid_part;

 public:
  void init(f32 width, f32 height) {
    nodes.clear();

    const f32 sqrtGrid = sqrt(uniformgrid_parts);
    const f32 col = width / sqrtGrid;
    const f32 row = height / sqrtGrid;

    for (f32 y = 0.0f; y < height; y += row) {
      for (f32 x = 0.0f; x < width; x += col) {
        Rect bounds(vec2(x, y), vec2(x + col, y + row));
        bounds.color = get_universal_current_color();
        ++universal_color_picker;
        nodes.emplace_back(std::make_unique<Node>(bounds));
      }
    }
    current_uniformgrid_part = uniformgrid_parts;
  };

  void input(const vector<T> &objects) {
    if (uniformgrid_parts != current_uniformgrid_part)
      init(screen_width, screen_height);
    data = objects;

    for (const auto &obj : data) {
      const auto id = obj.id;

      for (auto &node : nodes) {
        if (node->contains(id)) {
          node->insert(id);
        }
      }
    }
  }
  void color_objects(vector<vec4> &color) const {
    for (const auto &node : nodes) {
      node->color_objects(color);
    }
  }
  void draw_bounds() const {
    for (const auto &node : nodes) {
      node->draw_bounds();
    }
  }
  void get(vector<vector<s32>> &cont) const {
    for (const auto &node : nodes) {
      node->get(cont);
    }
  }
  void reset() {
    for (auto &node : nodes) node->index.clear();
  }
};

template <class T>
vector<T> UniformGrid<T>::data;

template <class T>
s32 UniformGrid<T>::current_uniformgrid_part{0};
