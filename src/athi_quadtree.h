#pragma once

#include "athi_typedefs.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "athi_rect.h"

struct Rect
{
  vec2 min,max;
  bool contains(u32 id) const;
  Rect(const glm::vec2 &min, const glm::vec2 &max);
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
