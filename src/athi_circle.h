#pragma once

#include <functional>
#include <mutex>
#include <vector>

#ifdef __APPLE__
#include <OpenCL/OpenCL.h>
#else
#include <CL/cl.h>
#endif

#define CIRCLE_NUM_VERTICES 36

#include "athi_transform.h"
#include "athi_typedefs.h"

#include "athi_quadtree.h"
#include "athi_utility.h"
#include "athi_voxelgrid.h"

extern vector<std::function<void()>> circle_update_call_buffer;

struct Athi_Circle {
  u32 id;
  vec2 pos;
  vec2 vel;
  vec2 acc;
  f32 mass;
  f32 radius{0.001f};
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Transform transform;

  void update();
  void border_collision();
  Athi_Circle() = default;
};

extern std::vector<std::unique_ptr<Athi_Circle>> circle_buffer;

struct Athi_Circle_Manager {
  // OPENCL
  // ////////////////////////////////////////////////////////////////////////////
  int err;  // error code returned from api calls
  char *kernel_source{nullptr};
  size_t global;  // global domain size for our calculation
  size_t local;   // local domain size for our calculation
  unsigned int begin;
  unsigned int end;
  bool gpu{true};
  std::vector<Athi_Circle> data;
  std::vector<Athi_Circle> results;

  cl_device_id device_id;     // compute device id
  cl_context context;         // compute context
  cl_command_queue commands;  // compute command queue
  cl_program program;         // compute program
  cl_kernel kernel;           // compute kernel

  cl_mem input;   // device memory used for the input array
  cl_mem output;  // device memory used for the output array
  ////////////////////////////////////////////////////////////////////////////////////////

  // Make sure it's thread safe
  bool clear_circles{false};
  std::mutex circle_buffer_function_mutex;
  std::mutex circle_buffer_mutex;
  std::vector<std::unique_ptr<Athi_Circle>> circle_buffer;
  void add_circle(Athi_Circle &circle);
  void add_circle_multiple(Athi_Circle &circle, int num);
  void update_circles();
  void draw_circles();
  Athi_Circle get_circle(u32 id);
  void set_color_circle_id(u32 id, const vec4 &color);
  //

  enum { POSITION, COLOR, TRANSFORM, NUM_BUFFERS };

  std::vector<mat4> transforms;
  std::vector<vec4> colors;

  u32 VAO;
  u32 VBO[NUM_BUFFERS];
  u32 shader_program;

  size_t transform_bytes_allocated{0};
  size_t color_bytes_allocated{0};

  Athi_Circle_Manager() = default;
  ~Athi_Circle_Manager();

  void init();
  void draw();
  void update();

  void collision_logNxN(size_t total, size_t begin, size_t end);
  void collision_quadtree(const std::vector<std::vector<int>> &cont, size_t begin, size_t end);
};

void update_circle_call(const std::function<void()> &f);

void attach_spring(Athi_Circle &a, Athi_Circle &b);
void delete_circles();
u32 get_num_circles();
void update_circles();
void draw_circles();
void add_circle_multiple(Athi_Circle &circle, int num);
void add_circle(Athi_Circle &circle);
void init_circle_manager();

bool collision_detection(const Athi_Circle &a, const Athi_Circle &b);
void collision_resolve(Athi_Circle &a, Athi_Circle &b);
void separate(Athi_Circle &a, Athi_Circle &b);
extern std::unique_ptr<Athi_Circle_Manager> athi_circle_manager;
