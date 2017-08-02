#pragma once

#include "athi_typedefs.h"
#include "athi_circle.h"
#include "athi_rect.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Athi_Quadtree
{
  u32 level{0};
  Rect bounds;
  std::unique_ptr<Athi_Quadtree> subnode[4]{nullptr};
  std::vector<u32> index;

  void split();
  void insert(u32 id);
  bool contains(u32 id);
  void init(const vec2& min, const vec2& max);
  void draw();
  void update();
  void get(std::vector<std::vector<u32> > &cont) const;
  Athi_Quadtree(u32 level, const Rect &bounds);
  Athi_Quadtree() = default;
};

static Athi_Quadtree athi_quadtree;

static void init_quadtree()
{
  athi_quadtree.init(vec2(-1,-1), vec2(1,1));
}

static void update_quadtree()
{
  athi_quadtree.update();
}
static void get_nodes_quadtree(std::vector<std::vector<u32> > &cont)
{
  athi_quadtree.get(cont);
}

static void draw_quadtree()
{
  athi_quadtree.draw();
}

static void reset_quadtree()
{
  //----------------------------------------------------------------
  // Sets bounds to the screens bounds and clears the quadtrees.
  //----------------------------------------------------------------
  athi_quadtree.index.clear();
  athi_quadtree.index.shrink_to_fit();

  athi_quadtree.subnode[0] = nullptr;
  athi_quadtree.subnode[1] = nullptr;
  athi_quadtree.subnode[2] = nullptr;
  athi_quadtree.subnode[3] = nullptr;
}
