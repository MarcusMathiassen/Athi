#pragma once

#include "athi_rect.h"
#include "athi_settings.h"

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

template <class T>
class Quadtree
{
public:
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
      if (o.x - r < max.x && o.x + r > min.x && o.y - r < max.y &&
          o.y + r > min.y)
        return true;
      return false;
    }
  };

  Quadtree(size_t level, const Rect &bounds) : bounds(bounds), level(level) {}
  Quadtree(size_t depth, size_t capacity, const glm::vec2 &min,
           const glm::vec2 &max)
      : max_depth(depth), max_capacity(capacity)
  {
    bounds.color = pastel_gray;
    bounds.min = min;
    bounds.max = max;
  }

  void draw()
  {
    if (sw)
    {
      sw->bounds.color = pastel_red;
      se->bounds.color = pastel_blue;
      nw->bounds.color = pastel_orange;
      ne->bounds.color = pastel_purple;
      sw->draw();
      se->draw();
      nw->draw();
      ne->draw();
      return;
    }

    // Only draw the nodes with objects in them.
    if (!indices.empty() && quadtree_show_only_occupied)
      draw_hollow_rect(bounds.min, bounds.max, bounds.color);
    else if (!quadtree_show_only_occupied)
      draw_hollow_rect(bounds.min, bounds.max, bounds.color);
  }

  void color_objects(std::vector<T> &obj) const
  {
    if (sw)
    {
      sw->bounds.color = pastel_red;
      se->bounds.color = pastel_gray;
      nw->bounds.color = pastel_orange;
      ne->bounds.color = pastel_pink;
      sw->color_objects(obj);
      se->color_objects(obj);
      nw->color_objects(obj);
      ne->color_objects(obj);
      return;
    }

    for (const auto id : indices)
      obj[id].color = bounds.color;
  }

  void input(const std::vector<T> &data_)
  {
    data = data_;
    indices.reserve(max_capacity);
    for (const auto &obj : data)
    {
      insert(obj.id);
    }
    data.clear();
  }

  void get(std::vector<std::vector<size_t>> &cont) const
  {
    if (sw)
    {
      sw->get(cont);
      se->get(cont);
      nw->get(cont);
      ne->get(cont);
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

    const float w = width * 0.5f;
    const float h = height * 0.5f;

    const Rect SW(glm::vec2(x, y), glm::vec2(x + w, y + h));
    const Rect SE(glm::vec2(x + w, y), glm::vec2(x + width, y + h));
    const Rect NW(glm::vec2(x, y + h), glm::vec2(x + w, y + height));
    const Rect NE(glm::vec2(x + w, y + h), glm::vec2(x + width, y + height));

    sw = std::make_unique<Quadtree<T>>(level + 1, SW);
    se = std::make_unique<Quadtree<T>>(level + 1, SE);
    nw = std::make_unique<Quadtree<T>>(level + 1, NW);
    ne = std::make_unique<Quadtree<T>>(level + 1, NE);
  }

  void insert(size_t id)
  {
    if (sw)
    {
      if (sw->contains(id))
        sw->insert(id);
      if (se->contains(id))
        se->insert(id);
      if (nw->contains(id))
        nw->insert(id);
      if (ne->contains(id))
        ne->insert(id);
      return;
    }

    indices.emplace_back(id);

    if (indices.size() > max_capacity && level < max_depth)
    {
      split();

      for (const auto index : indices)
      {
        if (sw->contains(index))
          sw->insert(index);
        if (se->contains(index))
          se->insert(index);
        if (nw->contains(index))
          nw->insert(index);
        if (ne->contains(index))
          ne->insert(index);
      }
      indices.clear();
    }
  }

  bool contains(size_t id) const
  {
    return bounds.contains(data[id].pos, data[id].radius);
  }

  std::vector<size_t> indices;
  static std::vector<T> data;

  std::unique_ptr<Quadtree<T>> sw;
  std::unique_ptr<Quadtree<T>> se;
  std::unique_ptr<Quadtree<T>> nw;
  std::unique_ptr<Quadtree<T>> ne;

  Rect bounds;
  size_t max_depth{5};
  size_t max_capacity{50};
  size_t level{0};
};

template <class T>
std::vector<T> Quadtree<T>::data;
