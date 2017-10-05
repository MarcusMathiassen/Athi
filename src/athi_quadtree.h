#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <array>

#include "athi_circle.h"

struct Rect
{
  glm::vec2 min, max;
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Rect() = default;
  Rect(const glm::vec2 &min, const glm::vec2 &max) : min(min), max(max) {}
  bool contains(const glm::vec2 &pos, float radius) const
  {
    const auto o = pos;
    const auto r = radius;
    if (o.x - r < max.x && o.x + r > min.x && o.y - r < max.y && o.y + r > min.y)
      return true;
    return false;
  }
};

template <class T>
class Quadtree
{
public:
  Quadtree(size_t level, const Rect &bounds) : level(level), bounds(bounds) {}
  Quadtree(size_t depth, size_t capacity, const glm::vec2 &min, const glm::vec2 &max) : max_depth(depth),                                                                                     max_capacity(capacity)
  {
    bounds.min = min;
    bounds.max = max;
  }

  void draw()
  {
    if (subnodes[0])
    {
      subnodes[0]->draw();
      subnodes[1]->draw();
      subnodes[2]->draw();
      subnodes[3]->draw();

      return;
    }

    // Only draw the nodes with objects in them.
    if (!indices.empty() && quadtree_show_only_occupied)
      draw_hollow_rect(bounds.min, bounds.max, bounds.color);
    else if (!quadtree_show_only_occupied)
      draw_hollow_rect(bounds.min, bounds.max, bounds.color);
  }
  void update()
  {
    indices.reserve(max_capacity);
    for (const auto &obj : athi_circle_manager->circle_buffer)
    {
      insert(obj.id);
    }
  }

  void get(std::vector<std::vector<size_t>> &cont) const
  {
    if (subnodes[0])
    {
      subnodes[0]->get(cont);
      subnodes[1]->get(cont);
      subnodes[2]->get(cont);
      subnodes[3]->get(cont);

      return;
    }

    if (!indices.empty())
      cont.emplace_back(indices);
  }

private:
  void split()
  {
    const glm::vec2 min = bounds.min;
    const glm::vec2 max = bounds.max;

    const float x = min.x;
    const float y = min.y;
    const float width = max.x - min.x;
    const float height = max.y - min.y;

    const float w = width * 0.5;
    const float h = height * 0.5;

    const Rect SW(glm::vec2(x, y), glm::vec2(x + w, y + h));
    const Rect SE(glm::vec2(x + w, y), glm::vec2(x + width, y + h));
    const Rect NW(glm::vec2(x, y + h), glm::vec2(x + w, y + height));
    const Rect NE(glm::vec2(x + w, y + h), glm::vec2(x + width, y + height));

    subnodes[0] = std::make_unique<Quadtree<T>>(level + 1, SW);
    subnodes[1] = std::make_unique<Quadtree<T>>(level + 1, SE);
    subnodes[2] = std::make_unique<Quadtree<T>>(level + 1, NW);
    subnodes[3] = std::make_unique<Quadtree<T>>(level + 1, NE);
  }

  void insert(size_t id)
  {
    if (subnodes[0])
    {
      if (subnodes[0]->contains(id))
        subnodes[0]->insert(id);
      if (subnodes[1]->contains(id))
        subnodes[1]->insert(id);
      if (subnodes[2]->contains(id))
        subnodes[2]->insert(id);
      if (subnodes[3]->contains(id))
        subnodes[3]->insert(id);
      return;
    }

    indices.emplace_back(id);

    if (indices.size() > max_capacity && level < max_depth)
    {
      split();

      for (const auto index : indices)
      {
        if (subnodes[0]->contains(index))
          subnodes[0]->insert(index);
        if (subnodes[1]->contains(index))
          subnodes[1]->insert(index);
        if (subnodes[2]->contains(index))
          subnodes[2]->insert(index);
        if (subnodes[3]->contains(index))
          subnodes[3]->insert(index);
      }
      indices.clear();
      indices.shrink_to_fit();
    }
  }

  bool contains(size_t id) const
  {
    return bounds.contains(athi_circle_manager->circle_buffer[id].pos, athi_circle_manager->circle_buffer[id].radius);
  }

  std::vector<T*> data; 
  std::array<std::unique_ptr<Quadtree<T>>, 4> subnodes;

  size_t max_depth;
  size_t max_capacity;
  size_t level = 0;
  std::vector<size_t> indices;
  Rect bounds;
};
