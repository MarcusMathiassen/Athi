#pragma once

#include "athi_circle.h"
#include "athi_rect.h"
#include "athi_typedefs.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Athi_Quadtree {
  u32 level{0};
  Rect bounds;
  std::unique_ptr<Athi_Quadtree> subnode[4]{nullptr};
  std::vector<u32> index;

  void split();
  void insert(u32 id);
  bool contains(u32 id);
  void init(const vec2 &min, const vec2 &max);
  void draw();
  void update();
  void get(std::vector<std::vector<u32>> &cont) const;
  Athi_Quadtree(u32 level, const Rect &bounds);
  Athi_Quadtree() = default;
};

extern std::unique_ptr<Athi_Quadtree> athi_quadtree;

void init_quadtree();
void update_quadtree();
void get_nodes_quadtree(std::vector<std::vector<u32>> &cont);
void draw_quadtree();
void reset_quadtree();
