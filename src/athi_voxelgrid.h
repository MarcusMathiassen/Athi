#pragma once

#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

#include "athi_rect.h"
#include "athi_typedefs.h"

struct Node
{
  Athi::Rect bounds;
  std::vector<size_t> index;
  Node(const Athi::Rect &r);
  void insert(size_t id);
  void get(std::vector<std::vector<size_t>> &cont);
  void draw() const;
  void color_objects();
  bool contains(size_t id);
};

class VoxelGrid
{
private:
  std::vector<std::unique_ptr<Node>> nodes;
  size_t current_voxelgrid_part{4};

public:
  void init();
  void update();
  void draw() const;
  void get(std::vector<std::vector<size_t>> &cont) const;
};

extern VoxelGrid athi_voxelgrid;

static void init_voxelgrid() { athi_voxelgrid.init(); }

static void update_voxelgrid() { athi_voxelgrid.update(); }
static void get_nodes_voxelgrid(std::vector<std::vector<size_t>> &cont)
{
  athi_voxelgrid.get(cont);
}

static void draw_voxelgrid() { athi_voxelgrid.draw(); }
