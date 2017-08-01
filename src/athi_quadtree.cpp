
#include "athi_quadtree.h"
#include "athi_circle.h"
#include "athi_settings.h"

#include <iostream>

Athi_Quadtree::Athi_Quadtree(u32 level, const Athi_Rect &bounds)
    : level(level),
      bounds(bounds),
      subnode{nullptr, nullptr, nullptr, nullptr} {
  index.reserve(quadtree_capacity);
}

void Athi_Quadtree::init(const vec2& min, const vec2& max)
{
  bounds.min = min;
  bounds.max = max;
  index.reserve(quadtree_capacity);
}

void Athi_Quadtree::update() {
  index.clear();
  index.shrink_to_fit();

  subnode[0] = nullptr;
  subnode[1] = nullptr;
  subnode[2] = nullptr;
  subnode[3] = nullptr;

  for (const auto &circle : athi_circle_manager.circle_buffer) {
    insert(circle.id);
  }
}

void Athi_Quadtree::split() {
  //----------------------------------------------------------------
  // Create subnodes and gives each its own quadrant.
  //----------------------------------------------------------------

  const vec2 min = bounds.min;
  const vec2 max = bounds.max;

  const f32 x = min.x;
  const f32 y = min.y;
  const f32 width = max.x - min.x;
  const f32 height = max.y - min.y;

  const f32 w = width * 0.5;
  const f32 h = height * 0.5;

  const Athi_Rect SW(vec2(x, y),     vec2(x+w,y+h));
  const Athi_Rect SE(vec2(x+w, y),   vec2(x+width,  y+h));
  const Athi_Rect NW(vec2(x, y+h),   vec2(x+w,      y+height));
  const Athi_Rect NE(vec2(x+w, y+h), vec2(x+width,  y+height));

  subnode[0] = std::make_unique<Athi_Quadtree>(level+1, SW);
  subnode[1] = std::make_unique<Athi_Quadtree>(level+1, SE);
  subnode[2] = std::make_unique<Athi_Quadtree>(level+1, NW);
  subnode[3] = std::make_unique<Athi_Quadtree>(level+1, NE);
}

void Athi_Quadtree::insert(u32 id) {
  //----------------------------------------------------------------
  // [1] Insert object into subnodes.
  // [2] If split, insert THIS nodes objects into the subnodes.
  // [3] Add object to this node.
  //----------------------------------------------------------------

  // If this subnode has split..
  if (subnode[0] != nullptr) {
    // Find the subnodes that contain it and insert it there.
    if (subnode[0]->contains(id)) subnode[0]->insert(id);
    if (subnode[1]->contains(id)) subnode[1]->insert(id);
    if (subnode[2]->contains(id)) subnode[2]->insert(id);
    if (subnode[3]->contains(id)) subnode[3]->insert(id);

    return;
  }

  // Add object to this node
  index.emplace_back(id);  // [3]

  // If it has NOT split..and NODE_CAPACITY is reached and we are not at MAX
  // LEVEL..
  if (index.size() > quadtree_capacity && level < quadtree_depth) {
    // Split into subnodes.
    split();

    // Go through all this nodes objects..
    for (const auto &index : index)  // [2]
    {
      // Go through all newly created subnodes..
      for (const auto &subnode : subnode) {
        // If they contain the objects..
        if (subnode->contains(index)) {
          // Insert the object into the subnode
          subnode->insert(index);
        }
      }
    }

    // Remove all indexes from THIS node
    index.clear();
    index.shrink_to_fit();
  }
}

void Athi_Quadtree::draw()
{
  //----------------------------------------------------------------
  // [1] Draw this nodes boundaries.
  // [2] Draw subnodes boundaries.
  //----------------------------------------------------------------

  if (subnode[0] != nullptr)  // [2]
  {
    // Continue down the tree
    subnode[0]->draw();
    subnode[1]->draw();
    subnode[2]->draw();
    subnode[3]->draw();

    return;
  }
  // Only draw the nodes with objects in them.
  //if (index.size() != 0) bounds.draw();  // [1]

  std::cout << "Bound drawn at: " << bounds.min.x  << "x" << bounds.min.y << " to " << bounds.max.x << "x" << bounds.max.y << std::endl;
  bounds.draw(GL_LINE_LOOP); // draw them all '
}

void Athi_Quadtree::get(std::vector<std::vector<u32> > &cont) const {
  //----------------------------------------------------------------
  // [1] Find the deepest level node.
  // [2] If there are indexes, add to container.
  //----------------------------------------------------------------

  // If this subnode has split..
  if (subnode[0] != nullptr)  // [1]
  {
    // Continue down the tree
    subnode[0]->get(cont);
    subnode[1]->get(cont);
    subnode[2]->get(cont);
    subnode[3]->get(cont);

    return;
  }

  // Insert indexes into our container
  if (!index.empty()) cont.emplace_back(index);  // [2]
}

bool Athi_Quadtree::contains(u32 id) {
  return bounds.contains(id);
}