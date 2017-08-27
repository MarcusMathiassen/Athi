#include "athi_voxelgrid.h"
#include "athi_circle.h"
#include "athi_settings.h"

#include <cmath>
#include <iostream>

VoxelGrid athi_voxelgrid;

Node::Node(const Athi::Rect &r) : bounds{r} {}

void Node::insert(int id) { index.emplace_back(id); }

void Node::get(std::vector<std::vector<int>> &cont) {
  cont.emplace_back(index);
  index.clear();
  index.shrink_to_fit();
}
void Node::draw() const {
  draw_hollow_rect(bounds.min, bounds.max, bounds.color);
}
void Node::color_objects() {
  for (const auto id : index) {
    athi_circle_manager->set_color_circle_id(id, bounds.color);
  }
}

bool Node::contains(int id) { return bounds.contains(id); }

void VoxelGrid::init() {
  //-----------------------------------------------------------------------------------
  // The nodes are cleared and given an element in the grid.
  //-----------------------------------------------------------------------------------

  for (auto &node : nodes) delete node;

  nodes.clear();
  nodes.shrink_to_fit();

  const f32 sqrtGrid = sqrt(voxelgrid_parts);
  const f32 col = 1.0f / sqrtGrid;
  const f32 row = 1.0f / sqrtGrid;

  for (f32 y = -1.0f; y < 1.0f; y += row) {
    for (f32 x = -1.0f; x < 1.0f; x += col) {
      Athi::Rect bounds(vec2(x, y), vec2(x + col, y + row));
      if (draw_debug) {
        bounds.color = get_universal_current_color();
        ++universal_color_picker;
      }
      Node *n = new Node(bounds);
      nodes.emplace_back(n);
    }
  }
  current_voxelgrid_part = voxelgrid_parts;
}

void VoxelGrid::update() {
  //-----------------------------------------------------------------------------------
  // Goes through every node and fills it with objects from the
  // main-container, any object that fits within the nodes boundaries will
  // be added to the nodes object-container.
  //-----------------------------------------------------------------------------------

  if (voxelgrid_parts != current_voxelgrid_part) init();

  for (const auto &circle : athi_circle_manager->circle_buffer) {
    const int id = circle->id;

    for (const auto &node : nodes)
      if (node->contains(id)) node->insert(id);
  }
}

void VoxelGrid::get(std::vector<std::vector<int>> &cont) const {
  // Color the objects with the color of the node
  for (const auto &node : nodes) {
    if (draw_debug) node->color_objects();
    node->get(cont);
  }
}

void VoxelGrid::draw() const {
  //-----------------------------------------------------------------------------------
  // Draws the nodes boundaries to screen and colors the objects within
  // each node with the nodes color.
  //-----------------------------------------------------------------------------------
  for (const auto &node : nodes) {
    node->draw();
  }
}
