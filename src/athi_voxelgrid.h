#pragma once

#include <vector>
#include <memory>

#include "athi_settings.h"
#include "athi_rect.h"
#include "athi_utility.h"

#define GLEW_STATIC
#include <GL/glew.h>

struct Rect {
  glm::vec2 min{0.0f, 0.0f}, max{0.0f, 0.0f};
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Rect(const glm::vec2 &min, const glm::vec2 &max) : min(min), max(max) {}
  bool contains(const glm::vec2 &pos, float radius) const {
    if (pos.x - radius < max.x && pos.x + radius > min.x && 
        pos.y - radius < max.y && pos.y + radius > min.y)
      return true;
    return false;
  }
};

template <class T>
class VoxelGrid {
protected:
  static std::vector<T> data;
private:
  struct Node : public VoxelGrid {
    Rect bounds;
    std::vector<int> index;
    Node(const Rect &r) : bounds{r} {}
    void insert(int id) { 
      index.emplace_back(id); 
    }
    void get(std::vector<std::vector<int>> &cont) {
      cont.emplace_back(index);
    }
    void draw_bounds() const {
        draw_hollow_rect(bounds.min, bounds.max, bounds.color);
    }
    void color_objects(std::vector<glm::vec4> &color) const {
      for (const auto i : index) {
        color[i] = bounds.color;
      }
    }

    bool contains(int id) { return bounds.contains(this->data[id].pos, this->data[id].radius); }
  };

  std::vector<std::unique_ptr<Node>> nodes;
  static int32_t current_voxelgrid_part;

 public:
  void init(float width, float height) {
    nodes.clear();

    const float sqrtGrid = sqrt(voxelgrid_parts);
    const float col = width / sqrtGrid;
    const float row = height / sqrtGrid;

    for (float y = 0.0f; y < height; y += row) {
      for (float x = 0.0f; x < width; x += col) {
        Rect bounds(glm::vec2(x, y), glm::vec2(x + col, y + row));
        bounds.color = get_universal_current_color();
        ++universal_color_picker;
        nodes.emplace_back(std::make_unique<Node>(bounds));
      }
    }
    current_voxelgrid_part = voxelgrid_parts;
  };

  void input(const std::vector<T> &objects) {
    if (voxelgrid_parts != current_voxelgrid_part) init(screen_width, screen_height);
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
  void color_objects(std::vector<glm::vec4> &color) const {
    for (const auto &node : nodes) {
      node->color_objects(color);
    }
  }
  void draw_bounds() const {
    for (const auto &node : nodes) {
      node->draw_bounds();
    }
  }
  void get(std::vector<std::vector<int>> &cont) const {
    for (const auto &node : nodes) {
        node->get(cont);
    }
  }
  void reset() {
    for (auto &node : nodes)
      node->index.clear();
  }
};

template <class T>
std::vector<T> VoxelGrid<T>::data;

template <class T>
int32_t VoxelGrid<T>::current_voxelgrid_part{0};