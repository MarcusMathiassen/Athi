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
#include "./Renderer/athi_rect.h" // draw_rect

template <class T> class Quadtree {
private:
  vector<s32> indices;
  static T *data;

  std::unique_ptr<Quadtree<T>> sw;
  std::unique_ptr<Quadtree<T>> se;
  std::unique_ptr<Quadtree<T>> nw;
  std::unique_ptr<Quadtree<T>> ne;

  Rect bounds;
  size_t level{0};

  constexpr void split() noexcept {
    const auto min = bounds.min;
    const auto max = bounds.max;

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

    if (indices.size() > max_capacity && level < max_depth) {
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

  constexpr bool contains(s32 id) const noexcept {
    return bounds.contains(data[id].pos, data[id].radius);
  }

public:

  static size_t max_depth;
  static size_t max_capacity;

  constexpr Quadtree(s32 level, const Rect &bounds) noexcept : bounds(bounds), level(level) {
    indices.reserve(max_capacity);
  }
  constexpr Quadtree(const vec2 &min, const vec2 &max) noexcept {
    indices.reserve(max_capacity);
    bounds.color = vec4(1,1,1,1);
    bounds.min = min;
    bounds.max = max;
  }
  constexpr void input(vector<T> &data_) noexcept {
    data = &data_[0];
    indices.reserve(max_capacity);
    for (const auto &obj : data_)
      insert(obj.id);
  }

  constexpr void color_neighbours(const T& p, const vec4& color) {
    if (sw) {
      if (sw->bounds.contains(p.pos, p.radius)) sw->color_neighbours(p, color);
      if (se->bounds.contains(p.pos, p.radius)) se->color_neighbours(p, color);
      if (nw->bounds.contains(p.pos, p.radius)) nw->color_neighbours(p, color);
      if (ne->bounds.contains(p.pos, p.radius)) ne->color_neighbours(p, color);
      return;
    }

    bounds.color = color;
  }

  constexpr void get_neighbours(vector<vector<s32>> &cont, const T& p) const noexcept {
    if (sw) {
      if (sw->bounds.contains(p.pos, p.radius)) sw->get_neighbours(cont, p);
      if (se->bounds.contains(p.pos, p.radius)) se->get_neighbours(cont, p);
      if (nw->bounds.contains(p.pos, p.radius)) nw->get_neighbours(cont, p);
      if (ne->bounds.contains(p.pos, p.radius)) ne->get_neighbours(cont, p);
      return;
    }

     if (!indices.empty())
      cont.emplace_back(indices);
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

  void draw_bounds(bool show_occupied_only, vec4 color) noexcept {
    if (sw) {
      sw->draw_bounds(show_occupied_only, color);
      se->draw_bounds(show_occupied_only, color);
      nw->draw_bounds(show_occupied_only, color);
      ne->draw_bounds(show_occupied_only, color);
      return;
    }

    if (!indices.empty() && show_occupied_only)
      draw_rect(bounds.min, bounds.max, color, true);
    else if (!show_occupied_only)
      draw_rect(bounds.min, bounds.max, color, true);
  }
};

template <class T> T *Quadtree<T>::data;
template <class T> size_t Quadtree<T>::max_depth;
template <class T> size_t Quadtree<T>::max_capacity;
