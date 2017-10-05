
#include "athi_quadtree.h"
#include "athi_circle.h"
#include "athi_settings.h"

std::unique_ptr<Quadtree> athi_quadtree;

Quadtree::Quadtree(size_t level, const Athi::Rect &bounds) : level(level), bounds(bounds) { indices.reserve(quadtree_capacity); }

void Quadtree::init(const vec2 &min, const vec2 &max) {
  bounds.min = min;
  bounds.max = max;
  bounds.color = pastel_gray;
  indices.reserve(quadtree_capacity);
}

void Quadtree::update() {
  subnodes[0].reset();
  subnodes[1].reset();
  subnodes[2].reset();
  subnodes[3].reset();
  indices.clear();
  indices.shrink_to_fit();
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

  const Athi::Rect SW(vec2(x, y), vec2(x + w, y + h));
  const Athi::Rect SE(vec2(x + w, y), vec2(x + width, y + h));
  const Athi::Rect NW(vec2(x, y + h), vec2(x + w, y + height));
  const Athi::Rect NE(vec2(x + w, y + h), vec2(x + width, y + height));

  subnodes[0] = std::make_unique<Quadtree>(level + 1, SW);
  subnodes[1] = std::make_unique<Quadtree>(level + 1, SE);
  subnodes[2] = std::make_unique<Quadtree>(level + 1, NW);
  subnodes[3] = std::make_unique<Quadtree>(level + 1, NE);

  if (draw_debug) {
    subnodes[0]->bounds.color = pastel_red;
    subnodes[1]->bounds.color = pastel_blue;
    subnodes[2]->bounds.color = pastel_orange;
    subnodes[3]->bounds.color = pastel_purple;
  }
}

void Quadtree::insert(int id) {
  //----------------------------------------------------------------
  // [1] Insert object into subnodes.
  // [2] If split, insert THIS nodes objects into the subnodes.
  // [3] Add object to this node.
  //----------------------------------------------------------------

  // If this subnodes has split..
  if (subnodes[0]) {
    // Find the subnodes that contain it and insert it there.
    if (subnodes[0]->contains(id)) subnodes[0]->insert(id);
    if (subnodes[1]->contains(id)) subnodes[1]->insert(id);
    if (subnodes[2]->contains(id)) subnodes[2]->insert(id);
    if (subnodes[3]->contains(id)) subnodes[3]->insert(id);

    return;
  }

  // Add object to this node
  indices.emplace_back(id);  // [3]

  // If it has NOT split..and NODE_CAPACITY is reached and we are not at MAX
  // LEVEL..
  const size_t indices_size = indices.size();
  if (indices_size > quadtree_capacity && level < quadtree_depth) {
    // Split into subnodes.
    split();

    // Go through all this nodes objects..
    for (const auto &indices : indices)  // [2]
    {
      if (subnodes[0]->contains(indices)) subnodes[0]->insert(indices);
      if (subnodes[1]->contains(indices)) subnodes[1]->insert(indices);
      if (subnodes[2]->contains(indices)) subnodes[2]->insert(indices);
      if (subnodes[3]->contains(indices)) subnodes[3]->insert(indices);
    }

    // Remove all indices from THIS node
    indices.clear();
    indices.shrink_to_fit();
  }
}

void Quadtree::draw() {
  //----------------------------------------------------------------
  // [1] Draw this nodes boundaries.
  // [2] Draw subnodes boundaries.
  //----------------------------------------------------------------

  if (subnodes[0])  // [2]
  {
    // Continue down the tree
    subnodes[0]->draw();
    subnodes[1]->draw();
    subnodes[2]->draw();
    subnodes[3]->draw();

    return;
  }

  // Only draw the nodes with objects in them.
  if (!indices.empty() && quadtree_show_only_occupied) {
    draw_hollow_rect(bounds.min, bounds.max, bounds.color);
  } else if (!quadtree_show_only_occupied) {
    draw_hollow_rect(bounds.min, bounds.max, bounds.color);
  }
}

void Quadtree::get(std::vector<std::vector<int>> &cont) const {
  //----------------------------------------------------------------
  // [1] Find the deepest level node.
  // [2] If there are indexes, add to container.
  //----------------------------------------------------------------

  // If this subnodes has split..
  if (subnodes[0])  // [1]
  {
    // Continue down the tree
    subnodes[0]->get(cont);
    subnodes[1]->get(cont);
    subnodes[2]->get(cont);
    subnodes[3]->get(cont);
    
    return;
  }

  // Color the balls in the same color as the boundaries
  if (draw_debug) {
    for (const auto &id : indices) {
      athi_circle_manager->set_color_circle_id(id, bounds.color);
    }
  }

  if (!indices.empty()) {
    cont.emplace_back(indices);  // [2]
  }
}

void Quadtree::retrieve(std::vector<int> &cont, const Athi::Rect &rect) const {
  //----------------------------------------------------------------
  // [1] Find the deepest level node.
  // [2] If there are indices, add to container.
  //----------------------------------------------------------------

  // If this subnode has split..
  if (subnodes[0])  // [1]
  {
    // Continue down the tree
    if (subnodes[0]->contain_rect(rect)) subnodes[0]->retrieve(cont, rect);
    if (subnodes[1]->contain_rect(rect)) subnodes[1]->retrieve(cont, rect);
    if (subnodes[2]->contain_rect(rect)) subnodes[2]->retrieve(cont, rect);
    if (subnodes[3]->contain_rect(rect)) subnodes[3]->retrieve(cont, rect);
    return;
  }

  // Add all indexes to our container
  for (const auto i : indices) cont.emplace_back(i);
}

bool Quadtree::contains(int id) { return bounds.contains(id); }
bool Quadtree::contain_rect(const Athi::Rect &rect) const { return bounds.contain_rect(rect); }

void init_quadtree() {
  athi_quadtree = std::make_unique<Quadtree>();
  athi_quadtree->init(vec2(-1, -1), vec2(1, 1));
}

void update_quadtree() { athi_quadtree->update(); }
void get_nodes_quadtree(std::vector<std::vector<int>> &cont) { athi_quadtree->get(cont); }

void retrieve_nodes_quadtree(std::vector<int> &cont, const Athi::Rect &rect) { athi_quadtree->retrieve(cont, rect); }

void draw_quadtree() {
   athi_quadtree->draw(); 
}

void reset_quadtree() {
  //----------------------------------------------------------------
  // Sets bounds to the screens bounds and clears the quadtrees.
  //----------------------------------------------------------------
  athi_quadtree->indices.clear();
  athi_quadtree->indices.shrink_to_fit();
  athi_quadtree->subnodes[0].reset();
  athi_quadtree->subnodes[1].reset();
  athi_quadtree->subnodes[2].reset();
  athi_quadtree->subnodes[3].reset();
}
