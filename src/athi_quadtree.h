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

#include "./Renderer/athi_rect.h" // draw_rect

#include <glm/vec2.hpp> // glm::vec2
#include <glm/vec4.hpp> // glm::vec4

#include <vector> // std::vector
#include <memory> // std::unique_ptr

class Quadtree
{
private:
  std::vector<int>   indices;

  static glm::vec2*  position;
  static float*      radius;

  std::unique_ptr<Quadtree>  sw;
  std::unique_ptr<Quadtree>  se;
  std::unique_ptr<Quadtree>  nw;
  std::unique_ptr<Quadtree>  ne;

  Rect  bounds;
  int   level {0};

  void split() noexcept
  {
    const glm::vec2 min = bounds.min;
    const glm::vec2 max = bounds.max;

    const float x = min.x;
    const float y = min.y;
    const float width = max.x - min.x;
    const float height = max.y - min.y;

    const float w = width * 0.5f;
    const float h = height * 0.5f;

    const Rect SW = Rect({x, y}, {x + w, y + h});
    const Rect SE = Rect({x + w, y}, {x + width, y + h});
    const Rect NW = Rect({x, y + h}, {x + w, y + height});
    const Rect NE = Rect({x + w, y + h}, {x + width, y + height});

    sw = std::make_unique<Quadtree>(level + 1, SW);
    se = std::make_unique<Quadtree>(level + 1, SE);
    nw = std::make_unique<Quadtree>(level + 1, NW);
    ne = std::make_unique<Quadtree>(level + 1, NE);
  }

  void insert(int id) noexcept
  {
    if (sw) {
      if (sw->contains(id)) sw->insert(id);
      if (se->contains(id)) se->insert(id);
      if (nw->contains(id)) nw->insert(id);
      if (ne->contains(id)) ne->insert(id);
      return;
    }

    indices.emplace_back(id);

    if (static_cast<int>(indices.size()) > max_capacity && level < max_depth) {
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

  bool contains(int id) const noexcept
  {
    return bounds.contains(position[id], radius[id]);
  }

public:

  static int max_depth;
  static int max_capacity;

  Quadtree() = default;

  Quadtree(int level, const Rect &bounds) noexcept : bounds(bounds), level(level)
  {
    indices.reserve(max_capacity);
  }

  Quadtree(const glm::vec2 &min, const glm::vec2 &max) noexcept
  {
    indices.reserve(max_capacity);
    bounds.color = glm::vec4(1,1,1,1);
    bounds.min = min;
    bounds.max = max;
  }

  void set_data(std::vector<glm::vec2>& position, std::vector<float>& radius) noexcept
  {
    this->position = &position[0];
    this->radius = &radius[0];
  }

  void input_range(int begin, int end) noexcept
  {
    indices.reserve(max_capacity);
    for (int i = begin; i < end; ++i)
    {
      insert(i);
    }
  }

  void color_neighbours(const glm::vec2& position, float radius, const glm::vec4& color)
  {
    if (sw) {
      if (sw->bounds.contains(position, radius)) sw->color_neighbours(position, radius, color);
      if (se->bounds.contains(position, radius)) se->color_neighbours(position, radius, color);
      if (nw->bounds.contains(position, radius)) nw->color_neighbours(position, radius, color);
      if (ne->bounds.contains(position, radius)) ne->color_neighbours(position, radius, color);
      return;
    }

    bounds.color = color;
  }

  void get_neighbours(std::vector<std::vector<int>> &cont, const glm::vec2& position, float radius) const noexcept
  {
    if (sw) {
      if (sw->bounds.contains(position, radius)) sw->get_neighbours(cont, position, radius);
      if (se->bounds.contains(position, radius)) se->get_neighbours(cont, position, radius);
      if (nw->bounds.contains(position, radius)) nw->get_neighbours(cont, position, radius);
      if (ne->bounds.contains(position, radius)) ne->get_neighbours(cont, position, radius);
      return;
    }

     if (!indices.empty()) {
      cont.emplace_back(indices);
     }
  }

  void get(std::vector<std::vector<int>> &cont) const noexcept
  {
    if (sw) {
      sw->get(cont);
      se->get(cont);
      nw->get(cont);
      ne->get(cont);
      return;
    }

    if (!indices.empty()) {
      cont.emplace_back(indices);
    }
  }

  void draw_bounds(bool show_occupied_only, glm::vec4 color) noexcept
  {
    if (sw) {
      sw->draw_bounds(show_occupied_only, color);
      se->draw_bounds(show_occupied_only, color);
      nw->draw_bounds(show_occupied_only, color);
      ne->draw_bounds(show_occupied_only, color);
      return;
    }

    if (!indices.empty() && show_occupied_only) {
      draw_rect(bounds.min, bounds.max, color, true);
    }
    else if (!show_occupied_only) {
      draw_rect(bounds.min, bounds.max, color, true);
    }
  }
};
