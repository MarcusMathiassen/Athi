#pragma once

#include "athi_typedefs.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "athi_rect.h"

class Athi_Quadtree
{
  u32 level{0};
  Athi_Rect bounds;
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
  Athi_Quadtree(u32 level, const Athi_Rect &bounds);
  Athi_Quadtree() = default;
};
