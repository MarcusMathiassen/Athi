
#pragma once
#include "athi_typedefs.h"

#include "./Utility/athi_globals.h"  // kPi, kGravitationalConstant

#include "athi_dispatch.h"
#include "athi_uniformgrid.h"

#include "athi_camera.h"
#include "athi_quadtree.h"
#include "athi_settings.h"
#include "athi_shader.h"
#include "athi_transform.h"

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

  void update(f32 dt) noexcept {
    // Update pos/vel/acc
    vel.x += acc.x * dt * time_scale * air_drag;
    vel.y += acc.y * dt * time_scale * air_drag;
    pos.x += vel.x * dt * time_scale;
    pos.y += vel.y * dt * time_scale;
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

struct ParticleSystem {
  u32 particle_count{0};
  f32 particle_density{1.0f};
  vector<Particle> particles;
  vector<Transform> transforms;
  vector<vec4> colors;
  vector<mat4> models;

  Dispatch pool;

  Shader shader;

  Quadtree<Particle> quadtree = Quadtree<Particle>(vec2(-1, -1), vec2(1, 1));
  UniformGrid<Particle> uniformgrid = UniformGrid<Particle>();

  enum { POSITION, COLOR, TRANSFORM, NUM_BUFFERS };
  u32 vao;
  u32 vbo[NUM_BUFFERS];
  size_t model_bytes_allocated{0};
  size_t color_bytes_allocated{0};

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
  void refresh_vertices() noexcept;
  void update() noexcept;
  void rebuild_vertices(u32 num_vertices) noexcept;
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
  void collision_quadtree(const vector<vector<s32>> &cont, size_t begin,
                          size_t end) noexcept;
  void add(const vec2 &pos, f32 radius,
           const vec4 &color = vec4(1, 1, 1, 1)) noexcept;

  void remove_all_with_id(const vector<s32> &ids) noexcept;
  void erase_all() noexcept;
};

// Returns a vector of ids of particles colliding with the input rectangle.
static vector<s32> get_particles_in_rect_basic(
    const vector<Particle> &particles, const vec2 &min,
    const vec2 &max) noexcept {
  vector<s32> vector_of_ids;

  // @Performance: Check for available tree structure used and use that instead.
  // Go through all the particles..
  for (const auto &particle : particles) {
    const auto o = particle.pos;

    // If the particle is inside the rectangle, add it to the output vector.
    if (o.x < max.x && o.x > min.x && o.y < max.y && o.y > min.y) {
      vector_of_ids.emplace_back(particle.id);
    }
  }

  return vector_of_ids;
}

// Returns a vector of ids of particles colliding with the input rectangle.
static vector<s32> get_particles_in_rect(const vector<Particle> &particles,
                                         const vec2 &min,
                                         const vec2 &max) noexcept {
  vector<s32> vector_of_ids;

  // @Performance: Check for available tree structure used and use that instead.
  // Go through all the particles..
  for (const auto &particle : particles) {
    const auto o = particle.pos;
    const auto r = particle.radius;

    // If the particle is inside the rectangle, add it to the output vector.
    if (o.x - r < max.x && o.x + r > min.x && o.y - r < max.y &&
        o.y + r > min.y) {
      vector_of_ids.emplace_back(particle.id);
    }
  }

  return vector_of_ids;
}

extern ParticleSystem particle_system;
