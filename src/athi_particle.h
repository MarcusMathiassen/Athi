// Copyright (c) 2018 Marcus Mathiassen

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.


#pragma once

#include "athi_typedefs.h"

#include "athi_dispatch.h"  // Dispatch

#include "athi_quadtree.h"     // Quadtree
#include "athi_uniformgrid.h"  // UniformGrid

#include "./Renderer/athi_renderer.h"    // Renderer
#include "athi_transform.h"  // Transform
#include "./Renderer/athi_texture.h"  // texture
#include "./Utility/threadsafe_container.h"  // ThreadSafe::vector
#include <mutex> // mutex

#ifdef __APPLE__
#include <OpenCL/OpenCL.h>
#else
#include <CL/cl.h>
#endif

struct Particle {
  s32 id{0};
  vec2 pos{0.0f, 0.0f};
  vec2 vel{0.0f, 0.0f};
  vec2 acc{0.0f, 0.0f};
  f32 mass{0.0f};
  f32 radius{0.0f};
  f32 torque{0.0f};

  void update(f32 dt) noexcept;
};

struct ParticleSystem {
  u32 particle_count{0};
  f32 particle_density{1.0f};

  // Data information
  size_t particles_vertices_size{0};

  std::mutex particles_mutex;

  vector<Particle> particles;
  vector<Transform> transforms;
  vector<vec4> colors;
  vector<mat4> models;

  vector<vector<s32>> tree_container;

  Renderer renderer;
  Texture tex;

  Dispatch pool;

  Quadtree<Particle> quadtree = Quadtree<Particle>(vec2(-1, -1), vec2(1, 1));
  UniformGrid<Particle> uniformgrid = UniformGrid<Particle>();

  // OPENCL
  // ///////////////////////////////////////////////////////
  s32 err;  // error code returned from api calls
  char *kernel_source{nullptr};
  size_t global_dim;  // global domain size for our calculation
  size_t local;       // local domain size for our calculation

  static constexpr bool gpu{true};
  vector<Particle> results;

  cl_device_id device_id;     // compute device id
  cl_context context;         // compute context
  cl_command_queue commands;  // compute command queue
  cl_program program;         // compute program
  cl_kernel kernel;           // compute kernel
  //////////////////////////////////////////////////////////

  void init() noexcept;
  void save_state() noexcept;
  void load_state() noexcept;
  void refresh_vertices() noexcept;
  void update(float dt) noexcept;
  void rebuild_vertices(u32 num_vertices) noexcept;
  void draw() noexcept;
  void opencl_init() noexcept;
  void draw_debug_nodes() noexcept;
  void update_gpu_buffers() noexcept;
  void update_collisions() noexcept;
  void opencl_naive() noexcept;
  void apply_n_body() noexcept;
  void threaded_buffer_update(size_t begin, size_t end) noexcept;
  bool collision_check(const Particle &a, const Particle &b) const noexcept;
  void collision_resolve(Particle &a, Particle &b) const noexcept;
  void separate(Particle &a, Particle &b) const noexcept;
  void collision_logNxN(size_t total, size_t begin, size_t end) noexcept;
  void collision_quadtree(const vector<vector<s32>> &cont, size_t begin,
                          size_t end) noexcept;
  void add(const vec2 &pos, f32 radius,
           const vec4 &color = vec4(1, 1, 1, 1)) noexcept;

  void remove_all_with_id(const vector<s32> &ids) noexcept;
  void erase_all() noexcept;

  vector<s32> get_neighbours(const Particle& p) const noexcept;

  // Returns a vector of ids of particles colliding with the input particle.
  vector<s32> get_particles_in_circle(const Particle &p) noexcept;
};

// Returns a vector of ids of particles colliding with the input rectangle.
vector<s32> get_particles_in_rect_basic(const vector<Particle> &particles,
                                        const vec2 &min,
                                        const vec2 &max) noexcept;

// Returns a vector of ids of particles colliding with the input rectangle.
vector<s32> get_particles_in_rect(const vector<Particle> &particles,
                                  const vec2 &min, const vec2 &max) noexcept;

extern ParticleSystem particle_system;
