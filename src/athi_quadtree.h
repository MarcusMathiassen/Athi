#pragma once

#include "athi_circle.h"
#include "athi_rect.h"
#include "athi_typedefs.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Quadtree
{
  Quadtree* subnodes {nullptr};

  int level{0};
  Rect bounds;
  std::vector<int> index;

  void split();
  void insert(int id);
  bool contains(int id);
  void clear();
  void init(const vec2 &min, const vec2 &max);
  void draw();
  void update();
  void get(std::vector<std::vector<int>> &cont) const;
  Quadtree(int level, const Rect &bounds);
  Quadtree() = default;
  ~Quadtree() { delete[] subnodes; }
};

extern std::unique_ptr<Quadtree> athi_quadtree;

void init_quadtree();
void update_quadtree();
void get_nodes_quadtree(std::vector<std::vector<int>> &cont);
void draw_quadtree();
void reset_quadtree();
