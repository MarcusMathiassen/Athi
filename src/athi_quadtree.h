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
  bool contain_rect(const Rect &rect) const;
  void clear();
  void init(const vec2 &min, const vec2 &max);
  void draw();
  void update();
  void get(std::vector<std::vector<int>> &cont) const;
  void retrieve(std::vector<int> &cont, const Rect &rect) const;    
  Quadtree(int level, const Rect &bounds);
  Quadtree() = default;
  ~Quadtree() { delete[] subnodes; }
};

extern std::unique_ptr<Quadtree> athi_quadtree;

void init_quadtree();
void update_quadtree();
void get_nodes_quadtree(std::vector<std::vector<int>> &cont);
void retrieve_nodes_quadtree(std::vector<int> &cont, const Rect &rect);
void draw_quadtree();
void reset_quadtree();
