#pragma once

#include "athi_rect.h"
#include "athi_settings.h"

#include <glm/vec2.hpp>
#include <memory>
#include <vector>

template <class T> class Quadtree {

private:
  std::vector<std::int32_t> indices;
  static T *data;

  std::unique_ptr<Quadtree<T>> sw;
  std::unique_ptr<Quadtree<T>> se;
  std::unique_ptr<Quadtree<T>> nw;
  std::unique_ptr<Quadtree<T>> ne;

  Rect bounds;
  std::int32_t level{0};

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

  constexpr void insert(std::int32_t id) noexcept {
    if (sw) {
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

    if (static_cast<std::int32_t>(indices.size()) > quadtree_capacity && level < quadtree_depth) {
      split();

      for (const auto index : indices) {
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

  constexpr bool contains(std::int32_t id) const noexcept { return bounds.contains(data[id].pos, data[id].radius); }

public:
  constexpr Quadtree(std::int32_t level, const Rect &bounds) noexcept : bounds(bounds), level(level) {}
  constexpr Quadtree(const glm::vec2 &min, const glm::vec2 &max) noexcept {
    bounds.color = pastel_gray;
    bounds.min_pos = min;
    bounds.max_pos = max;
  }
  constexpr void input(std::vector<T> &data_) noexcept {
    data = &data_[0];
    indices.reserve(quadtree_capacity);
    for (const auto &obj : data_)
      insert(obj.id);
  }

  constexpr void get(std::vector<std::vector<std::int32_t>> &cont) const noexcept {
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

  void color_objects(std::vector<glm::vec4> &color) const noexcept {
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
