#pragma once

#include <vector>

#include "athi_settings.h"
#include "athi_rect.h"

#define GLEW_STATIC
#include <GL/glew.h>

struct Node;

template <class T>
class VoxelGrid {
protected:
  static std::vector<T> data;
private:
  std::vector<std::unique_ptr<Node>> nodes;
  size_t current_voxelgrid_part{4};
 public:
  void init() {
    //-----------------------------------------------------------------------------------
    // The nodes are cleared and given an element in the grid.
    //-----------------------------------------------------------------------------------
    nodes.clear();

    const float sqrtGrid = sqrt(voxelgrid_parts);
    const float col = 1.0f / sqrtGrid;
    const float row = 1.0f / sqrtGrid;

    for (float y = -1.0f; y < 1.0f; y += row) {
      for (float x = -1.0f; x < 1.0f; x += col) {
        Rect bounds(glm::vec2(x, y), glm::vec2(x + col, y + row));
        bounds.color = get_universal_current_color();
        ++universal_color_picker;
        nodes.emplace_back(std::make_unique<Node>(bounds));
      }
    }
    current_voxelgrid_part = voxelgrid_parts;
  };

  //-----------------------------------------------------------------------------------
  // Goes through every node and fills it with objects from the
  // main-container, any object that fits within the nodes boundaries will
  // be added to the nodes object-container.
  //-----------------------------------------------------------------------------------
  void input(const std::vector<T> &objects) {
    if (voxelgrid_parts != current_voxelgrid_part) init();
    data = objects;

    for (const auto &obj : data) {
      const auto id = obj.id;

    for (const auto &node : nodes)
        if (node->contains(id)) node->insert(id);
    }
  }
  void draw() const {
    //-----------------------------------------------------------------------------------
    // Draws the nodes boundaries to screen and colors the objects within
    // each node with the nodes color.
    //-----------------------------------------------------------------------------------
    for (const auto &node : nodes) {
      node->draw();
    }
  }
  void get(std::vector<std::vector<int>> &cont) const {
    for (const auto &node : nodes) {
        node->get(cont);
    }
  }
};


struct Node : public VoxelGrid {
  struct Rect {
    glm::vec2 min, max;
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    Rect() = default;
    Rect(const glm::vec2 &min, const glm::vec2 &max) : min(min), max(max) {}
    bool contains(const glm::vec2 &pos, float radius) const {
      const auto o = pos;
      const auto r = radius;
      if (o.x - r < max.x && o.x + r > min.x && o.y - r < max.y &&
          o.y + r > min.y)
        return true;
      return false;
    }
  };

  Rect bounds;
  std::vector<int> index;
  Node(const Rect &r) : bounds{r} {}
  void insert(int id) { index.emplace_back(id); }
  void get(std::vector<std::vector<int>> &cont) {
    cont.emplace_back(index);
    index.clear();
  }
  void draw() const {
      draw_hollow_rect(bounds.min, bounds.max, bounds.color);
  }

  bool contains(int id) { return bounds.contains(this->data[id].pos, this->data[id].radius); }
};

template <class T>
std::vector<T> VoxelGrid<T>::data;
