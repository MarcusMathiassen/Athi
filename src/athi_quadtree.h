#pragma once

#include "athi_rect.h"
#include "athi_settings.h"

#include <array>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <memory>
#include <vector>

template <class T> class Quadtree {
public:
  struct Rect {
    glm::vec2 min_pos{0.0f, 0.0f}, max_pos{0.0f, 0.0f};
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    Rect() = default;
    Rect(const glm::vec2 &min, const glm::vec2 &max) : min_pos(min), max_pos(max) {}
    bool contains(const glm::vec2 &pos, float radius) const {
      if (pos.x - radius < max_pos.x && pos.x + radius > min_pos.x &&
          pos.y - radius < max_pos.y && pos.y + radius > min_pos.y)
        return true;
      return false;
    }
  };

  constexpr Quadtree(std::int32_t level, const Rect &bounds) noexcept : bounds(bounds), level(level) {}
  constexpr Quadtree(const glm::vec2 &min, const glm::vec2 &max) noexcept {
    bounds.color = pastel_gray;
    bounds.min_pos = min;
    bounds.max_pos = max;
  }

  void draw_bounds() {
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

  void color_objects(std::vector<glm::vec4> &color) const {
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

  void input(const std::vector<T> &data_) {
    data = data_;
    indices.reserve(quadtree_capacity);
    for (const auto &obj : data) {
      insert(obj.id);
    }
  }

  void get(std::vector<std::vector<std::int32_t>> &cont) const {
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

private:
  void split() {
    const glm::vec2 min = bounds.min_pos;
    const glm::vec2 max = bounds.max_pos;

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

  void insert(std::int32_t id) {
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

    if (static_cast<std::int32_t>(indices.size()) > quadtree_capacity &&
        level < quadtree_depth) {
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

  bool contains(std::int32_t id) const {
    return bounds.contains(data[id].pos, data[id].radius);
  }

  std::vector<std::int32_t> indices;
  static std::vector<T> data;

  std::unique_ptr<Quadtree<T>> sw;
  std::unique_ptr<Quadtree<T>> se;
  std::unique_ptr<Quadtree<T>> nw;
  std::unique_ptr<Quadtree<T>> ne;

  Rect bounds;
  std::int32_t level{0};
};

template <class T> std::vector<T> Quadtree<T>::data;
