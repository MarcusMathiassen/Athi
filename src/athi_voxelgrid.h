#pragma once

#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

#include "athi_rect.h"
#include "athi_typedefs.h"

struct Node {
  Rect bounds;
  std::vector<int> index;
  Node(const Rect &r);
  void insert(int id);
  void get(std::vector<std::vector<int>> &cont);
  void draw() const;
  void color_objects();
  bool contains(int id);
};

class VoxelGrid {
private:
  std::vector<Node*> nodes;
  int current_voxelgrid_part{4};

public:
  void init();
  void update();
  void draw() const;
  void get(std::vector<std::vector<int>> &cont) const;
};

extern VoxelGrid athi_voxelgrid;

static void init_voxelgrid() { athi_voxelgrid.init(); }

static void update_voxelgrid() { athi_voxelgrid.update(); }
static void get_nodes_voxelgrid(std::vector<std::vector<int>> &cont) {
  athi_voxelgrid.get(cont);
}

static void draw_voxelgrid() { athi_voxelgrid.draw(); }
