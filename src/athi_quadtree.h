#pragma once

#include "athi_typedefs.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Rect
{
  vec2 min,max;
  Rect(const vec2& min, const vec2& max) : min(min), max(max) {}
  bool contains(u32 id);
  Rect() = default;
};

class Athi_Quadtree
{
  u32 level{0};
  Rect bounds;
  std::unique_ptr<Athi_Quadtree> subnode[4]{nullptr};
  std::vector<u32> index;

  void split();
  void insert(u32 id);
  bool contains(u32 id);

 public:
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
