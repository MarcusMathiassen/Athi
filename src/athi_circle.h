#pragma once

#include "athi_typedefs.h"
#include "athi_transform.h"

#include "athi_utility.h"

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
  void draw() const;
  void borderCollision();
  void init();

  Athi_Circle() = default;
};


struct Athi_Circle_Manager
{
  enum { POSITION, COLOR, TRANSFORM, NUM_BUFFERS };

  u32 num_circles{0};
  std::vector<Athi_Circle> circle_buffer;

  std::vector<mat4> transforms;
  std::vector<vec4> colors;

  // Mesh
  u32 VAO;
  u32 VBO[NUM_BUFFERS];
  size_t transform_bytes_allocated{0};
  size_t color_bytes_allocated{0};

  // Shader
  u32 shader_program;

  Athi_Circle_Manager() = default;
  ~Athi_Circle_Manager();

  void init();
  void draw();
  void update();
};

void delete_circles();
u32  get_num_circles();
void update_circles();
void draw_circles();
void addCircle(Athi_Circle &circle);
void init_circle_manager();

static bool collisionDetection(const Athi_Circle &a, const Athi_Circle &b);
static void collisionResolve(Athi_Circle &a, Athi_Circle &b);
static void separate(Athi_Circle &a, Athi_Circle &b);
static void collision_logNxN(size_t begin, size_t end);

static Athi_Circle_Manager athi_circle_manager;
