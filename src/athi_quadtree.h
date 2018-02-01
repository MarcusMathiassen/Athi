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

#include "athi_rect.h" // Rect
#include "athi_settings.h" // quadtree_show_only_occupied, etc.

template <class T> class Quadtree {
private:
  vector<s32> indices;
  static T *data;

  std::unique_ptr<Quadtree<T>> sw;
  std::unique_ptr<Quadtree<T>> se;
  std::unique_ptr<Quadtree<T>> nw;
  std::unique_ptr<Quadtree<T>> ne;

  Rect bounds;
  s32 level{0};

  constexpr void split() noexcept {
    const auto min = bounds.min_pos;
    const auto max = bounds.max_pos;

    const auto x = min.x;
    const auto y = min.y;
    const auto width = max.x - min.x;
    const auto height = max.y - min.y;

    const auto w = width * 0.5f;
    const auto h = height * 0.5f;

    const auto SW = Rect({x, y}, {x + w, y + h});
    const auto SE = Rect({x + w, y}, {x + width, y + h});
    const auto NW = Rect({x, y + h}, {x + w, y + height});
    const auto NE = Rect({x + w, y + h}, {x + width, y + height});

    sw = std::make_unique<Quadtree<T>>(level + 1, SW);
    se = std::make_unique<Quadtree<T>>(level + 1, SE);
    nw = std::make_unique<Quadtree<T>>(level + 1, NW);
    ne = std::make_unique<Quadtree<T>>(level + 1, NE);
  }

  constexpr void insert(s32 id) noexcept {
    if (sw) {
      if (sw->contains(id)) sw->insert(id);
      if (se->contains(id)) se->insert(id);
      if (nw->contains(id)) nw->insert(id);
      if (ne->contains(id)) ne->insert(id);
      return;
    }

    indices.emplace_back(id);

    if (static_cast<s32>(indices.size()) > quadtree_capacity && level < quadtree_depth) {
      split();

      for (const auto index : indices) {
        if (sw->contains(index)) sw->insert(index);
        if (se->contains(index)) se->insert(index);
        if (nw->contains(index)) nw->insert(index);
        if (ne->contains(index)) ne->insert(index);
      }
      indices.clear();
    }
  }

  constexpr bool contains(s32 id) const noexcept { return bounds.contains(data[id].pos, data[id].radius); }

public:
  constexpr Quadtree(s32 level, const Rect &bounds) noexcept : bounds(bounds), level(level) {}
  constexpr Quadtree(const vec2 &min, const vec2 &max) noexcept {
    bounds.color = pastel_gray;
    bounds.min_pos = min;
    bounds.max_pos = max;
  }
  constexpr void input(vector<T> &data_) noexcept {
    data = &data_[0];
    indices.reserve(quadtree_capacity);
    for (const auto &obj : data_)
      insert(obj.id);
  }

  constexpr void get(vector<vector<s32>> &cont) const noexcept {
    if (sw) {
      sw->get(cont);
      se->get(cont);
      nw->get(cont);
      ne->get(cont);
      return;
    }

    if (!indices.empty())
      cont.emplace_back(indices);
  }

  void draw_bounds() noexcept {
    if (sw) {
      sw->bounds.color = sw_color;
      se->bounds.color = se_color;
      nw->bounds.color = nw_color;
      ne->bounds.color = ne_color;
      sw->draw_bounds();
      se->draw_bounds();
      nw->draw_bounds();
      ne->draw_bounds();
      return;
    }

    if (!indices.empty() && quadtree_show_only_occupied)
      draw_hollow_rect(bounds.min_pos, bounds.max_pos, bounds.color);
    else if (!quadtree_show_only_occupied)
      draw_hollow_rect(bounds.min_pos, bounds.max_pos, bounds.color);
  }

  void color_objects(vector<vec4> &color) const noexcept {
    if (sw) {
      sw->bounds.color = sw_color;
      se->bounds.color = se_color;
      nw->bounds.color = nw_color;
      ne->bounds.color = ne_color;
      sw->color_objects(color);
      se->color_objects(color);
      nw->color_objects(color);
      ne->color_objects(color);
      return;
    }

    for (const auto id : indices)
      color[id] = bounds.color;
  }
};

template <class T> T *Quadtree<T>::data;
