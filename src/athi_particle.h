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

#include "Utility/fixed_size_types.h"  // u32, s32, etc.

#include "athi_dispatch.h"  // Dispatch

#include "athi_quadtree.h"  // Quadtree
#include "athi_uniformgrid.h"  // UniformGrid

#include "./Renderer/athi_renderer.h"  // Renderer

#include "athi_transform.h"  // Transform

#include "./Renderer/athi_texture.h"  // texture

#include <mutex>  // mutex
#include <functional>

#ifdef __APPLE__
#include <OpenCL/OpenCL.h>
#else
#include <CL/cl.h>
#endif

#include <vector>  // std::vector
#include <glm/vec2.hpp>  // glm::vec2
#include <glm/vec4.hpp>  // glm::vec4

struct Particle
{
  typedef glm::vec2 vec2;

  s32        id          {0};
  vec2       pos         {0.0f, 0.0f};
  vec2       vel         {0.0f, 0.0f};
  vec2       acc         {0.0f, 0.0f};
  f32        mass        {0.0f};
  f32        radius      {0.0f};
  f32        torque      {0.0f};

  void update(f32 dt) noexcept;
};

struct ParticleSystem
{
  u32   particle_count    {0};
  f32   particle_density  {1.0f};

  // Data information
  size_t particles_vertices_size{0};

  std::mutex              particles_mutex;
  std::vector<Particle>   particles;

  std::vector<Transform>  transforms;
  std::vector<glm::vec4>  colors;
  std::vector<glm::mat4>  models;

  std::vector<std::vector<s32>> tree_container;

  Renderer    renderer;
  Texture     tex;

  Dispatch    pool;

  Quadtree<Particle>      quadtree;
  UniformGrid<Particle>   uniformgrid;

  // OPENCL
  // ///////////////////////////////////////////////////////
  s32 err;  // error code returned from api calls
  char *kernel_source{nullptr};
  size_t global_dim;  // global domain size for our calculation
  size_t local;       // local domain size for our calculation

  static constexpr bool gpu{true};
  std::vector<Particle> results;

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
  void update_data() noexcept;
  void gpu_buffer_update() noexcept;
  void update_collisions() noexcept;
  void opencl_naive() noexcept;
  void apply_n_body() noexcept;
  void threaded_buffer_update(size_t begin, size_t end) noexcept;
  bool collision_check(const Particle &a, const Particle &b) const noexcept;
  void collision_resolve(Particle &a, Particle &b) const noexcept;
  void separate(Particle &a, Particle &b) const noexcept;
  void collision_logNxN(size_t total, size_t begin, size_t end) noexcept;
  void collision_quadtree(const std::vector<std::vector<s32>> &cont, size_t begin,
                          size_t end) noexcept;
  void add(const glm::vec2 &pos, f32 radius,
           const glm::vec4 &color = glm::vec4(1, 1, 1, 1)) noexcept;

  void remove_all_with_id(const std::vector<s32> &ids) noexcept;
  void erase_all() noexcept;

  void pull_towards_point(const glm::vec2& point) noexcept;

  std::mutex buffered_call_mutex;
  std::vector<std::function<void()>> buffered_call_buffer;
  void buffered_call(std::function<void()>&& f) noexcept;
  void execute_buffered_calls() noexcept;

  void set_particles_color(std::vector<s32> ids, const glm::vec4& color) noexcept;

  std::vector<s32> get_neighbours(const Particle& p) const noexcept;

  // Returns a std::vector of ids of particles colliding with the input particle.
  std::vector<s32> get_particles_in_circle(const Particle &p) noexcept;
};

// Returns a std::vector of ids of particles colliding with the input rectangle.
std::vector<s32> get_particles_in_rect_basic(const std::vector<Particle> &particles,
                                        const glm::vec2 &min,
                                        const glm::vec2 &max) noexcept;

// Returns a std::vector of ids of particles colliding with the input rectangle.
std::vector<s32> get_particles_in_rect(const std::vector<Particle> &particles,
                                  const glm::vec2 &min, const glm::vec2 &max) noexcept;

extern ParticleSystem particle_system;
