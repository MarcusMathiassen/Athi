#include "athi_voxelgrid.h"
#include "athi_settings.h"
#include "athi_circle.h"

#include <cmath>
#include <iostream>

Node::Node(const Rect &r) : bounds{r} {}

void Node::insert(const u32 id)
{
  index.emplace_back(id);
}

void Node::get(std::vector<std::vector<u32>> &cont)
{
  cont.emplace_back(index);
  index.clear();
  index.shrink_to_fit();
}
void Node::draw() const
{
  const f32 width = bounds.max.x - bounds.min.x;
  const f32 height = bounds.max.y - bounds.min.y;
  draw_rect(bounds.min, width, height, bounds.color, GL_LINE_LOOP);
}
void Node::color_objects()
{
  for (const auto &id : index)
    circle_buffer[id].color = bounds.color;
}

bool Node::contain(const u32 id) { return bounds.contains(id); }


void Athi_Voxel_Grid::init()
{
  //-----------------------------------------------------------------------------------
  // The nodes are cleared and given an element in the grid.
  //-----------------------------------------------------------------------------------

  nodes.clear();
  nodes.shrink_to_fit();

  f32 sqrtGrid = sqrt(voxelgrid_parts);
  f32 col = 1.0f / sqrtGrid;
  f32 row = 1.0f / sqrtGrid;

  for (f32 y = -1.0f; y < 1.0f; y += row)
  {
    for (f32 x = -1.0f; x < 1.0f; x += col)
    {
      Rect bounds(vec2(x, y), vec2(x + col, y + row));
      nodes.emplace_back(std::make_unique<Node>(bounds));
    }
  }
}

void Athi_Voxel_Grid::update()
{
  //-----------------------------------------------------------------------------------
  // Goes through every node and fills it with objects from the
  // main-container, any object that fits within the nodes boundaries will
  // be added to the nodes object-container.
  //-----------------------------------------------------------------------------------

  for (const auto &circle : circle_buffer)
  {
    u32 id = circle.id;

    for (const auto &node : nodes)
      if (node->contain(id))
        node->insert(id);
  }
}

void Athi_Voxel_Grid::get(std::vector<std::vector<u32>> &cont) const
{
  // Color the objects with the color of the node
  for (const auto &node : nodes)
  {
    node->color_objects();
    node->get(cont);
  }
}

void Athi_Voxel_Grid::draw() const
{
  //-----------------------------------------------------------------------------------
  // Draws the nodes boundaries to screen and colors the objects within
  // each node with the nodes color.
  //-----------------------------------------------------------------------------------
  for (const auto &node : nodes)
    node->draw();
}
