#pragma once

#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>

#include "athi_typedefs.h"
#include "athi_rect.h"

struct Node
{
  Rect bounds;
  std::vector<u32> index;
  Node(const Rect &r);
  void insert(const u32 id);
  void get(std::vector<std::vector<u32>> &cont);
  void draw() const;
  void color_objects();
  bool contain(const u32 id);
};

class Athi_Voxel_Grid
{
 private:
  std::vector<std::unique_ptr<Node> > nodes;

 public:
  void init();
  void update();
  void draw() const;
  void get(std::vector<std::vector<u32>> &cont) const;
};

static Athi_Voxel_Grid athi_voxelgrid;

static void init_voxelgrid()
{
  athi_voxelgrid.init();
}

static void update_voxelgrid()
{
  athi_voxelgrid.update();
}
static void get_nodes_voxelgrid(std::vector<std::vector<u32> > &cont)
{
  athi_voxelgrid.get(cont);
}

static void draw_voxelgrid()
{
  athi_voxelgrid.draw();
}