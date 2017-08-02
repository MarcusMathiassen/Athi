#pragma once

#include "athi_typedefs.h"
#include "athi_transform.h"

#include "athi_utility.h"
#include "athi_quadtree.h"
#include "athi_voxelgrid.h"

#include <mutex>
#include <vector>

#define CIRCLE_NUM_VERTICES 36

struct Athi_Circle
{
  u32 id;
  vec2 pos{0, 0};
  vec2 vel{0, 0};

  f32 radius{0.001f};
  f32 mass{1.0f};

  Transform transform;

  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

  void update();
  void borderCollision();

  Athi_Circle() = default;
};

extern std::vector<std::unique_ptr<Athi_Circle> > circle_buffer;

struct Athi_Circle_Manager
{
  enum { POSITION, COLOR, TRANSFORM, NUM_BUFFERS };

  std::vector<mat4> transforms;
  std::vector<vec4> colors;

  u32 VAO;
  u32 VBO[NUM_BUFFERS];
  size_t transform_bytes_allocated{0};
  size_t color_bytes_allocated{0};

  u32 shader_program;

  Athi_Circle_Manager() = default;
  ~Athi_Circle_Manager();

  void init();
  void draw();
  void update();

  void collision_logNxN(size_t begin, size_t end);
  void collision_quadtree(const std::vector<std::vector<u32> > &cont, size_t begin, size_t end);
};

void delete_circles();
u32  get_num_circles();
void update_circles();
void draw_circles();
void add_circle(Athi_Circle &circle);
void init_circle_manager();

bool collisionDetection(const Athi_Circle &a, const Athi_Circle &b);
void collisionResolve(Athi_Circle &a, Athi_Circle &b);
void separate(Athi_Circle &a, Athi_Circle &b);
extern std::unique_ptr<Athi_Circle_Manager> athi_circle_manager;
