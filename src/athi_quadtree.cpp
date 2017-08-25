
#include "athi_quadtree.h"
#include "athi_circle.h"
#include "athi_settings.h"

#include <cstdlib>

std::unique_ptr<Quadtree> athi_quadtree;

Quadtree::Quadtree(int level, const Rect &bounds)
    : level(level),
      bounds(bounds){
  index.reserve(quadtree_capacity);
}

void Quadtree::init(const vec2 &min, const vec2 &max) {
  bounds.min = min;
  bounds.max = max;
  bounds.color = pastel_gray;
  index.reserve(quadtree_capacity);
}

void Quadtree::update() {
  index.clear();
  index.shrink_to_fit();
  delete[] subnodes;
  subnodes = nullptr;
  clear();
  for (const auto &circle : athi_circle_manager->circle_buffer) {
    insert(circle->id);
  }
}

void Quadtree::split() {
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

  const Rect SW(vec2(x, y), vec2(x + w, y + h));
  const Rect SE(vec2(x + w, y), vec2(x + width, y + h));
  const Rect NW(vec2(x, y + h), vec2(x + w, y + height));
  const Rect NE(vec2(x + w, y + h), vec2(x + width, y + height));
  
  subnodes = new Quadtree[4];
  
  subnodes[0] = Quadtree(level + 1, SW);
  subnodes[1] = Quadtree(level + 1, SE);
  subnodes[2] = Quadtree(level + 1, NW);
  subnodes[3] = Quadtree(level + 1, NE);

  subnodes[0].bounds.color = pastel_red;
  subnodes[1].bounds.color = pastel_gray;
  subnodes[2].bounds.color = pastel_orange;
  subnodes[3].bounds.color = pastel_pink;
}

void Quadtree::insert(int id)
{
  //----------------------------------------------------------------
  // [1] Insert object into subnodes.
  // [2] If split, insert THIS nodes objects into the subnodes.
  // [3] Add object to this node.
  //----------------------------------------------------------------

  // If this subnodes has split..
  if (subnodes)
  {
    // Find the subnodes that contain it and insert it there.
    if (subnodes[0].contains(id)) subnodes[0].insert(id);
    if (subnodes[1].contains(id)) subnodes[1].insert(id);
    if (subnodes[2].contains(id)) subnodes[2].insert(id);
    if (subnodes[3].contains(id)) subnodes[3].insert(id);

    return;
  }

  // Add object to this node
  index.emplace_back(id); // [3]

  // If it has NOT split..and NODE_CAPACITY is reached and we are not at MAX
  // LEVEL..
  const int index_size = (int)index.size();
  if (index_size > quadtree_capacity && level < quadtree_depth)
  {
    // Split into subnodes.
    split();

    // Go through all this nodes objects..
    for (const auto &index : index) // [2]
    {
      if (subnodes[0].contains(index)) subnodes[0].insert(index);
      if (subnodes[1].contains(index)) subnodes[1].insert(index);
      if (subnodes[2].contains(index)) subnodes[2].insert(index);
      if (subnodes[3].contains(index)) subnodes[3].insert(index);
    }

    // Remove all indexes from THIS node
    index.clear();
    index.shrink_to_fit();
  }
}

void Quadtree::draw() {
  //----------------------------------------------------------------
  // [1] Draw this nodes boundaries.
  // [2] Draw subnodes boundaries.
  //----------------------------------------------------------------

  if (subnodes) // [2]
  {
    // Continue down the tree
    subnodes[0].draw();
    subnodes[1].draw();
    subnodes[2].draw();
    subnodes[3].draw();

    return;
  }

  // Only draw the nodes with objects in them.
  if (!index.empty() && quadtree_show_only_occupied)
  {
    draw_hollow_rect(bounds.min, bounds.max, bounds.color);
  }
  else if (!quadtree_show_only_occupied) draw_hollow_rect(bounds.min, bounds.max, bounds.color);

}

void Quadtree::get(std::vector<std::vector<int>> &cont) const {
  //----------------------------------------------------------------
  // [1] Find the deepest level node.
  // [2] If there are indexes, add to container.
  //----------------------------------------------------------------

  // If this subnodes has split..
  if (subnodes) // [1]
  {
    // Continue down the tree
    subnodes[0].get(cont);
    subnodes[1].get(cont);
    subnodes[2].get(cont);
    subnodes[3].get(cont);

    return;
  }

  // Color the balls in the same color as the boundaries
  for (const auto &id : index) {
    athi_circle_manager->set_color_circle_id(id, bounds.color);
  }

  // Insert indexes into our container
  if (!index.empty())
    cont.emplace_back(index); // [2]
}

bool Quadtree::contains(int id) { return bounds.contains(id); }


// Go down to the leaf nodes and delete the nodes
void Quadtree::clear() {
  if (subnodes)
  {
    // Continue down the tree
    subnodes[0].clear();
    subnodes[1].clear();
    subnodes[2].clear();
    subnodes[3].clear();
    return;
  }
  delete[] subnodes;
}

void init_quadtree() {
  athi_quadtree = std::make_unique<Quadtree>();
  athi_quadtree->init(vec2(-1, -1), vec2(1, 1));
}

void update_quadtree()
{
  athi_quadtree->update();
}
void get_nodes_quadtree(std::vector<std::vector<int>> &cont)
{
  athi_quadtree->get(cont);
}

void draw_quadtree() { athi_quadtree->draw(); }

void reset_quadtree() {
  //----------------------------------------------------------------
  // Sets bounds to the screens bounds and clears the quadtrees.
  //----------------------------------------------------------------
  athi_quadtree->index.clear();
  athi_quadtree->index.shrink_to_fit();
  athi_quadtree->clear();
}
