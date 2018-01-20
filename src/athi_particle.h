
#pragma once

#include "athi_dispatch.h"
#include "athi_quadtree.h"
#include "athi_settings.h"
#include "athi_shader.h"
#include "athi_transform.h"
#include "athi_typedefs.h"
#include "athi_voxelgrid.h"

#ifdef __APPLE__
#include <OpenCL/OpenCL.h>
#else
#include <CL/cl.h>
#endif

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>

struct Particle {
  int id{0};
  glm::vec2 pos{0.0f, 0.0f};
  glm::vec2 vel{0.0f, 0.0f};
  glm::vec2 acc{0.0f, 0.0f};
  float mass{0.0f};
  float radius{0.0f};

  void update() noexcept {
    // Apply gravity
    if (physics_gravity) {
      vel.y -= gravity_force * timestep;
    }

    // Update pos/vel/acc
    vel.x += acc.x * timestep * time_scale;
    vel.y += acc.y * timestep * time_scale;
    pos.x += vel.x * timestep * time_scale;
    pos.y += vel.y * timestep * time_scale;
    acc *= 0;

    if (border_collision) {
      // Border collision
      if (pos.x < 0 + radius) {
        pos.x = 0 + radius;
        vel.x = -vel.x * collision_energy_loss;
      }
      if (pos.x > screen_width - radius) {
        pos.x = screen_width - radius;
        vel.x = -vel.x * collision_energy_loss;
      }
      if (pos.y < 0 + radius) {
        pos.y = 0 + radius;
        vel.y = -vel.y * collision_energy_loss;
      }
      if (pos.y > screen_height - radius) {
        pos.y = screen_height - radius;
        vel.y = -vel.y * collision_energy_loss;
      }
    }
  }
};

struct ParticleManager {
  static constexpr std::int32_t num_verts{36};
  std::vector<Particle> particles;
  std::vector<Transform> transforms;
  std::vector<glm::vec4> colors;
  std::vector<glm::mat4> models;

  Dispatch pool;

  Shader shader;

  Quadtree<Particle> quadtree =
      Quadtree<Particle>(glm::vec2(-1, -1), glm::vec2(1, 1));
  VoxelGrid<Particle> voxelgrid = VoxelGrid<Particle>();

  enum { POSITION, COLOR, TRANSFORM, NUM_BUFFERS };
  std::uint32_t vao;
  std::uint32_t vbo[NUM_BUFFERS];
  std::size_t model_bytes_allocated{0};
  std::size_t color_bytes_allocated{0};

  // OPENCL
  // ///////////////////////////////////////////////////////
  std::int32_t err;  // error code returned from api calls
  char *kernel_source{nullptr};
  std::size_t global_dim;  // global domain size for our calculation
  std::size_t local;       // local domain size for our calculation

  static constexpr bool gpu{true};
  std::vector<Particle> results;

  cl_device_id device_id;     // compute device id
  cl_context context;         // compute context
  cl_command_queue commands;  // compute command queue
  cl_program program;         // compute program
  cl_kernel kernel;           // compute kernel

  cl_mem input;   // device memory used for the input array
  cl_mem output;  // device memory used for the output array
  //////////////////////////////////////////////////////////

  void init() noexcept;
  void update() noexcept;
  void draw() noexcept;
  void opencl_init() noexcept;
  void draw_debug_nodes() noexcept;
  void update_gpu_buffers() noexcept;
  void update_collisions() noexcept;
  void opencl_naive() noexcept;
  void apply_n_body() noexcept;
  bool collision_check(const Particle &a, const Particle &b) const noexcept;
  void collision_resolve(Particle &a, Particle &b) const noexcept;
  void separate(Particle &a, Particle &b) const noexcept;
  void collision_logNxN(size_t total, size_t begin, size_t end) noexcept;
  void collision_quadtree(const std::vector<std::vector<std::int32_t>> &cont,
                          size_t begin, size_t end) noexcept;
  void add(const glm::vec2 &pos, float radius,
           const glm::vec4 &color = glm::vec4(1, 1, 1, 1)) noexcept;
  void erase_all() noexcept;
};

extern ParticleManager particle_manager;
