
#pragma once

#include "athi_dispatch.h"
#include "athi_quadtree.h"
#include "athi_settings.h"
#include "athi_shader.h"
#include "athi_camera.h"
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
  float mass{0.0f};
  float radius{0.0f};

  //Transform transform; // this is a 36 byte struct

  void update(float dt) noexcept {
    // Apply gravity
    if (physics_gravity) {
      vel.y -= gravity_force * dt * time_scale;
    }

    // Update pos/vel/acc
    pos.x += vel.x * dt * time_scale;
    pos.y += vel.y * dt * time_scale;

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
  std::vector<std::int32_t> id;
  std::vector<glm::vec2> position;
  std::vector<glm::vec2> velocity;
  std::vector<float> radius;
  std::vector<float> mass;
  std::vector<glm::vec4> color;

  Shader particle_shader;

  std::uint32_t sprite_atlas;

  std::vector<Transform> transform;
  std::vector<glm::mat4> model;
  enum { POSITION_BUFFER, TEXCOORD_BUFFER, COLOR_BUFFER, INDICES_BUFFER, TRANSFORM_BUFFER, NUM_BUFFERS };
  std::uint32_t vao, vbo[NUM_BUFFERS];

  void init() noexcept {

    particle_shader.init("ParticleSystem.init()");
    particle_shader.load_from_file("../Resources/billboard_particle_shader.vert", ShaderType::Vertex);
    particle_shader.load_from_file("../Resources/billboard_particle_shader.frag", ShaderType::Fragment);
    particle_shader.bind_attrib("position");
    particle_shader.bind_attrib("texcoord");
    particle_shader.bind_attrib("color");
    particle_shader.link();
    particle_shader.add_uniform("transform");
    particle_shader.add_uniform("res");
    particle_shader.add_uniform("tex");

    glGenVertexArrays(1, &vao);
    glGenBuffers(NUM_BUFFERS, vbo);

    const std::uint16_t indices[6] = {0, 1, 2, 0, 2, 3};

    const GLfloat positions[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
    };

    const GLfloat texcoords[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    const GLfloat colors[] = {
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
    };

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void *)0);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[COLOR_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[INDICES_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  }

  void update_gpu_buffers() noexcept {

  }

  void update(float timestep) noexcept {
    const auto num_particles = id.size();

    // @Performance: this can be done in parallel
    for (std::size_t i = 0; i < num_particles; ++i) {

      auto& pos = position[i];
      auto& vel = velocity[i];

      pos.x += vel.x * timestep * time_scale;
      pos.y += vel.y * timestep * time_scale;
    }
  }

  void draw() noexcept {
    glBindVertexArray(vao);
    particle_shader.bind();
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, sprite_atlas);

    const auto proj = camera.get_ortho_projection();
    mat4 trans = proj *  Transform().get_model();

    particle_shader.setUniform("transform", trans);
    particle_shader.setUniform("res", screen_width, screen_height);
    particle_shader.setUniform("tex", 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
  }

  void add_particle(const Particle& p) noexcept {
    id.emplace_back(p.id);
    position.emplace_back(p.pos);
    velocity.emplace_back(p.vel);
    radius.emplace_back(p.radius);
    mass.emplace_back(p.mass);
  }

  void reserve(std::size_t size) noexcept {
    id.reserve(size);
    position.reserve(size);
    velocity.reserve(size);
    radius.reserve(size);
    mass.reserve(size);
  }

  void resize(std::size_t size) noexcept {
    id.resize(size);
    position.resize(size);
    velocity.resize(size);
    radius.resize(size);
    mass.resize(size);
  }

  void clear() noexcept {
    id.clear();
    position.clear();
    velocity.clear();
    radius.clear();
    mass.clear();
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
  void collision_quadtree(const std::vector<std::vector<std::int32_t>> &cont, size_t begin, size_t end) noexcept;
  void add(const glm::vec2 &pos, float radius,
           const glm::vec4 &color = glm::vec4(1, 1, 1, 1)) noexcept;

  void remove_all_with_id(const std::vector<std::int32_t>& ids) noexcept;
  void erase_all() noexcept;
};

// Returns a vector of ids of particles colliding with the input rectangle.
static std::vector<std::int32_t> get_particles_in_rect_basic(const std::vector<Particle>& particles, const glm::vec2& min, const glm::vec2& max) noexcept {
  std::vector<std::int32_t> vector_of_ids;

  // @Performance: Check for available tree structure used and use that instead.
  // Go through all the particles..
  for (const auto& particle: particles) {
    
    const auto o = particle.pos;

    // If the particle is inside the rectangle, add it to the output vector.
    if (o.x < max.x && o.x > min.x && o.y < max.y && o.y > min.y) {
      vector_of_ids.emplace_back(particle.id);
    }
  }
  
  return vector_of_ids;
}

// Returns a vector of ids of particles colliding with the input rectangle.
static std::vector<std::int32_t> get_particles_in_rect(const std::vector<Particle>& particles, const glm::vec2& min, const glm::vec2& max) noexcept {
  std::vector<std::int32_t> vector_of_ids;

  // @Performance: Check for available tree structure used and use that instead.
  // Go through all the particles..
  for (const auto& particle: particles) {
    
    const auto o = particle.pos;
    const auto r = particle.radius;

    // If the particle is inside the rectangle, add it to the output vector.
    if (o.x - r < max.x && o.x + r > min.x && o.y - r < max.y && o.y + r > min.y) {
      vector_of_ids.emplace_back(particle.id);
    }
  }
  
  return vector_of_ids;
}

extern ParticleManager particle_manager;
