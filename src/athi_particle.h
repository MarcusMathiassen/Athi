
#pragma once

#include "athi_dispatch.h"
#include "athi_quadtree_v2.h"
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

inline static auto get_begin_and_end(int i, int total, int threads) noexcept {
  const int parts = total / threads;
  const int leftovers = total % threads;
  const int begin = parts * i;
  int end = parts * (i + 1);
  if (i == threads - 1) end += leftovers;
  return std::tuple<std::size_t, std::size_t>{begin, end};
};

static HSV LerpHSV (HSV a, HSV b, float t)
{
 // Hue interpolation
 float h;
 float d = b.h - a.h;
 if (a.h > b.h)
 {
 // Swap (a.h, b.h)
 float h3 = b.h;
 b.h = a.h;
 a.h = h3;
 
 d = -d;
 t = 1 - t;
 }
 
 if (d > 0.5) // 180deg
 {
 a.h = a.h + 1; // 360deg
 h = ( a.h + t * (b.h - a.h) ); // 360deg
 }
 if (d <= 0.5) // 180deg
 {
 h = a.h + t * d;
 }
 
 // Interpolates the rest
 return HSV
 (
 h, // H
 a.s + t * (b.s-a.s), // S
 a.v + t * (b.v-a.v), // V
 a.a + t * (b.a-a.a) // A
 );
}

static glm::vec4 color_by_acceleration(const glm::vec4& min_color, const glm::vec4& max_color, const glm::vec2& acc) noexcept {

  // Get the HSV equivalent
  const float mg = sqrt(acc.x * acc.x + acc.y * acc.y);

  const auto c1 = rgb_to_hsv(min_color);
  const auto c2 = rgb_to_hsv(max_color);

  const auto c3 = LerpHSV(c1,c2,mg);
  return getHSV(c3.h, c3.s, c3.v, c3.a);
}

struct Particle {
  int id{0};
  glm::vec2 pos{0.0f, 0.0f};
  glm::vec2 vel{0.0f, 0.0f};
  glm::vec2 acc{0.0f, 0.0f};
  float mass{0.0f};
  float radius{0.0f};

  void update(float dt) noexcept {

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
  std::uint32_t particles_count{0};           // Number of particles

  // Particle structure:
  std::vector<std::int32_t> id;     // each particles unique id
  std::vector<glm::vec2> position;  // 
  std::vector<glm::vec2> velocity;  // 
  std::vector<float> radii;         //
  std::vector<float> mass;          // @TODO: is this needed? probably not at the moment
  std::vector<glm::vec4> color;     //
  //

  Shader particle_shader;

  enum { POSITION_BUFFER, COLOR_BUFFER, NUM_BUFFERS };
  std::uint32_t vao, vbo[NUM_BUFFERS];
  std::size_t color_bytes_allocated{0};
  std::size_t position_bytes_allocated{0};

  Quadtree_v2 quadtree = Quadtree_v2(glm::vec2(-1, -1), glm::vec2(1, 1));
  VoxelGrid<Particle> voxelgrid = VoxelGrid<Particle>();

  Dispatch pool;

  void init() noexcept {

    particle_shader.init("ParticleSystem.init()");
    particle_shader.load_from_file("billboard_particle_shader.vert", ShaderType::Vertex);
    particle_shader.load_from_file("billboard_particle_shader.frag", ShaderType::Fragment);
    particle_shader.bind_attrib("position");
    particle_shader.bind_attrib("color");
    particle_shader.link();

    glGenVertexArrays(1, &vao);
    glGenBuffers(NUM_BUFFERS, vbo);

    glBindVertexArray(vao);

    // POSITIONS
    glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION_BUFFER]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void *)0);

    // COLORS
    glBindBuffer(GL_ARRAY_BUFFER, vbo[COLOR_BUFFER]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void *)0);
  }

  void update_gpu_buffers() noexcept {

    glBindVertexArray(vao);
    profile p("ParticleSystem::update_gpu_buffers(GPU buffer update)");

    // Update the gpu buffers in case of more particles..
    glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION_BUFFER]);
    const auto position_bytes_needed = sizeof(glm::vec2) * particles_count;
    if (position_bytes_needed > position_bytes_allocated) {
      glBufferData(GL_ARRAY_BUFFER, position_bytes_needed, &position[0], GL_STREAM_DRAW);
      position_bytes_allocated = position_bytes_needed;
    } else {
      glBufferSubData(GL_ARRAY_BUFFER, 0, position_bytes_allocated, &position[0]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo[COLOR_BUFFER]);
    const auto color_bytes_needed = sizeof(glm::vec4) * particles_count;
    if (color_bytes_needed > color_bytes_allocated) {
      glBufferData(GL_ARRAY_BUFFER, color_bytes_needed, &color[0], GL_STREAM_DRAW);
      color_bytes_allocated = color_bytes_needed;
    } else {
      glBufferSubData(GL_ARRAY_BUFFER, 0, color_bytes_allocated, &color[0]);
    }
  }

  void update_collisions() noexcept {
    // reset the values
    comparisons = 0;
    resolutions = 0;

    // // Use a tree to partition the data
    std::vector<std::vector<std::int32_t>> tree_container;
    switch (tree_type) {
      using Tree = TreeType;
      case Tree::Quadtree: {
        quadtree = Quadtree_v2({-1.0f, -1.0f}, {1.0, 1.0});
        {
          profile p("Quadtree_v2.input()");
          quadtree.input(particles_count, &position[0], &radii[0]);
        }
        {
          profile p("Quadtree_v2.get()");
          quadtree.get(tree_container);
        }
        break;
      }
      // case Tree::UniformGrid: {
      //   {
      //     profile p("voxelgrid.reset()");
      //     voxelgrid.reset();
      //   }
      //   {
      //     profile p("voxelgrid.input()");
      //     voxelgrid.input(particles);
      //   }
      //   {
      //     profile p("voxelgrid.get()");
      //     voxelgrid.get(tree_container);
      //   }
      //   break;
      //}
      default: /* Using naive approch */
        break;
    }

    // if (openCL_active && particles_count >= 256) {
    //   opencl_naive();
    //   return;
    // }

    if (use_multithreading && variable_thread_count != 0) {
      if constexpr (os == OS::Apple) {
        threadpool_solution = ThreadPoolSolution::AppleGCD;
      } else {
        threadpool_solution = ThreadPoolSolution::Dispatch;
      }
    } else
      threadpool_solution = ThreadPoolSolution::None;

    profile p("ParticleSystem::update::circle_collision");

    const std::size_t total = particles_count;
    const std::size_t container_total = tree_container.size();

    switch (threadpool_solution) {
      using Threads = ThreadPoolSolution;
      case Threads::AppleGCD: {
        switch (tree_type) {
          using Tree = TreeType;
          case Tree::Quadtree: [[fallthrough]];
          case Tree::UniformGrid: {
            dispatch_apply(
                variable_thread_count,
                dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0),
                ^(size_t i) {
                  const auto[begin, end] = get_begin_and_end(
                      i, container_total, variable_thread_count);
                  collision_quadtree(tree_container, begin, end);
                });
          } break;
          case Tree::None: {
            dispatch_apply(
                variable_thread_count,
                dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0),
                ^(size_t i) {
                  const auto[begin, end] =
                      get_begin_and_end(i, total, variable_thread_count);
                  collision_logNxN(total, begin, end);
                });
          } break;
        }
      } break;

      case Threads::Dispatch: {
        switch (tree_type) {
          using Tree = TreeType;
          case Tree::Quadtree: [[fallthrough]];
          case Tree::UniformGrid: {
            std::vector<std::future<void>> results(variable_thread_count);
            for (int i = 0; i < variable_thread_count; ++i) {
              const auto[begin, end] =
                  get_begin_and_end(i, container_total, variable_thread_count);
              results[i] = pool.enqueue(&ParticleSystem::collision_quadtree,
                                        this, tree_container, begin, end);
            }
            for (auto &&res : results) res.get();
          } break;

          case Tree::None: {
            std::vector<std::future<void>> results(variable_thread_count);
            for (int i = 0; i < variable_thread_count; ++i) {
              const auto[begin, end] =
                  get_begin_and_end(i, total, variable_thread_count);
              results[i] = pool.enqueue(&ParticleSystem::collision_logNxN, this,
                                        total, begin, end);
            }
            for (auto &&res : results) res.get();
          } break;
        }
      } break;

      case Threads::None: {
        switch (tree_type) {
          using Tree = TreeType;
          case Tree::Quadtree: [[fallthrough]];
          case Tree::UniformGrid: { collision_quadtree(tree_container, 0, container_total); } break;
          case Tree::None: { collision_logNxN(total, 0, total); } break;
        }
      } break;
    }
  }

  void draw_debug_nodes() noexcept {
    profile p("ParticleSystem::draw_debug_nodes");

    if (draw_debug) {
      switch (tree_type) {
        using TT = TreeType;
        case TT::Quadtree: {
          if (color_particles) quadtree.color_objects(color);
          if (draw_nodes) quadtree.draw_bounds();
        } break;

        case TT::UniformGrid: {
          if (color_particles) voxelgrid.color_objects(color);
          if (draw_nodes) voxelgrid.draw_bounds();
        } break;

        case TT::None: {
        } break;
      }
    }
  }

  // Particles are moved forward by deltatime
  void update(float dt) noexcept {

    {
    float this_sample_timestep = 0;
    for (int i = 0; i < physics_samples; ++i) {
      const auto start = glfwGetTime();

      if (circle_collision) {
        update_collisions();
      } 

      {
      profile p("ParticleSystem::update(particles::update)");
      // @Performance: this can be done in parallel
      for (std::size_t i = 0; i < particles_count; ++i) {

        auto& pos = position[i];
        auto& vel = velocity[i];

        // Apply gravity
        if (physics_gravity) {
          vel.y -= (gravity_force*0.0001f) * dt * time_scale;
        }

        // Update position
        pos.x += vel.x * dt * time_scale;
        pos.y += vel.y * dt * time_scale;


        // Border collision
        if (border_collision) {

          auto& radius = radii[i];

          if (pos.x < -1.0 + radius) { pos.x = -1.0 + radius; vel.x = -vel.x * collision_energy_loss; } // left
          if (pos.x >  1.0 - radius) { pos.x =  1.0 - radius; vel.x = -vel.x * collision_energy_loss; } // right
          if (pos.y < -1.0 + radius) { pos.y = -1.0 + radius; vel.y = -vel.y * collision_energy_loss; } // bottom
          if (pos.y >  1.0 - radius) { pos.y =  1.0 - radius; vel.y = -vel.y * collision_energy_loss; } // top
        }
      }
      }
      const auto ms_spent_this_sample = (glfwGetTime() - start) * 1000.0;
      const auto desired_frametime = (1000.0 / 60.0);
      // ms used / 16.6667ms per frame / samples
      this_sample_timestep += (ms_spent_this_sample / desired_frametime);
    }
    timestep = this_sample_timestep;
    }

    if (use_gravitational_force) {
      profile p("ParticleSystem::apply_n_body()");
      apply_n_body();
    }
  }

  void draw() noexcept {
    glBindVertexArray(vao);
    particle_shader.bind();
    glDrawArrays(GL_POINTS, 0, particles_count);
  }

  bool collision_check(std::int32_t a, std::int32_t b) const noexcept {
    // Local variables
    const auto ax = position[a].x;
    const auto ay = position[a].y;
    const auto bx = position[b].x;
    const auto by = position[b].y;
    const auto ar = radii[a];
    const auto br = radii[b];

    // square collision check
    if (ax - ar < bx + br && ax + ar > bx - br && ay - ar < by + br &&
        ay + ar > by - br) {
      const auto dx = bx - ax;
      const auto dy = by - ay;

      const auto sum_radius = ar + br;
      const auto sqr_radius = sum_radius * sum_radius;

      const auto distance_sqrd = (dx * dx) + (dy * dy);

      // circle collision check
      return distance_sqrd < sqr_radius;
    }
    return false;
  }

  // Separates two s32ersecting circles.
  void separate(std::int32_t a, std::int32_t b) noexcept {
    // Local variables
    const auto a_pos = position[a];
    const auto b_pos = position[b];
    const auto ar = radii[a];
    const auto br = radii[b];

    const auto collision_depth = (ar + br) - glm::distance(b_pos, a_pos);

    const auto dx = b_pos.x - a_pos.x;
    const auto dy = b_pos.y - a_pos.y;

    // contact angle
    const auto collision_angle = atan2(dy, dx);
    const auto cos_angle = cosf(collision_angle);
    const auto sin_angle = sinf(collision_angle);

    // TODO: could this be done using a normal vector and just inverting it?
    // amount to move each ball
    const auto a_move_x = -collision_depth * 0.5f * cos_angle;
    const auto a_move_y = -collision_depth * 0.5f * sin_angle;
    const auto b_move_x = collision_depth * 0.5f * cos_angle;
    const auto b_move_y = collision_depth * 0.5f * sin_angle;

    // store the new move values
    glm::vec2 a_pos_move;
    glm::vec2 b_pos_move;

    // Make sure they dont moved beyond the border
    if (a_pos.x + a_move_x >= -1.0f + ar && a_pos.x + a_move_x <= 1.0 - ar)  a_pos_move.x += a_move_x;
    if (a_pos.y + a_move_y >= -1.0f + ar && a_pos.y + a_move_y <= 1.0 - ar) a_pos_move.y += a_move_y;
    if (b_pos.x + b_move_x >= -1.0f + br && b_pos.x + b_move_x <= 1.0 - br)  b_pos_move.x += b_move_x;
    if (b_pos.y + b_move_y >= -1.0f + br && b_pos.y + b_move_y <= 1.0 - br) b_pos_move.y += b_move_y;

    // Update positions
    position[a] += a_pos_move;
    position[b] += b_pos_move;
  }

  // Collisions response between two circles with varying radius and mass.
  void collision_resolve(std::int32_t a, std::int32_t b) noexcept {
    // Local variables
    const auto dx = position[b].x - position[a].x;
    const auto dy = position[b].y - position[a].y;
    const auto a_vel = velocity[a];
    const auto b_vel = velocity[b];
    const auto vdx = b_vel.x - a_vel.x;
    const auto vdy = b_vel.y - a_vel.y;
    const auto m1 = mass[a];
    const auto m2 = mass[b];

    // Should the circles intersect. Seperate them. If not the next
    // calculated values will be off.
    separate(a, b);

    // A negative 'd' means the circles velocities are in opposite
    // directions
    const auto d = dx * vdx + dy * vdy;

    // And we don't resolve collisions between circles moving away from
    // eachother
    if (d < 0) {
      const auto norm = glm::normalize(glm::vec2(dx, dy));
      const auto tang = glm::vec2{norm.y * -1.0, norm.x};
      const auto scal_norm_1 = glm::dot(norm, a_vel);
      const auto scal_norm_2 = glm::dot(norm, b_vel);
      const auto scal_tang_1 = glm::dot(tang, a_vel);
      const auto scal_tang_2 = glm::dot(tang, b_vel);

      const auto scal_norm_1_after = (scal_norm_1 * (m1 - m2) + 2.0f * m2 * scal_norm_2) / (m1 + m2);
      const auto scal_norm_2_after = (scal_norm_2 * (m2 - m1) + 2.0f * m1 * scal_norm_1) / (m1 + m2);
      const auto scal_norm_1_after_vec = norm * scal_norm_1_after;
      const auto scal_norm_2_after_vec = norm * scal_norm_2_after;
      const auto scal_norm_1_vec = tang * scal_tang_1;
      const auto scal_norm_2_vec = tang * scal_tang_2;

      // Update velocities
      velocity[a] = (scal_norm_1_vec + scal_norm_1_after_vec) * collision_energy_loss;
      velocity[b] = (scal_norm_2_vec + scal_norm_2_after_vec) * collision_energy_loss;
    }
  }

  void gravitational_force(std::int32_t a, std::int32_t b) {
    const float x1 = position[a].x;
    const float y1 = position[a].y;
    const float x2 = position[b].x;
    const float y2 = position[b].y;
    const float m1 = mass[a];
    const float m2 = mass[b];

    const float dx = x2 - x1;
    const float dy = y2 - y1;
    const float d = sqrt(dx * dx + dy * dy);

    if (d > 1e-4f) {
      const float angle = atan2(dy, dx);
      const float G = gravitational_constant;
      const float F = G * m1 * m2 / d * d;

      velocity[a].x += F * cos(angle);
      velocity[a].y += F * sin(angle);
    }
  }

  void apply_n_body() noexcept {
    for (std::size_t i = 0; i < particles_count; ++i) {
      for (std::size_t j = 0; j < particles_count; ++j) {
        gravitational_force(i, j);
      }
    }
  }

  // (N-1)*N/2
  void collision_logNxN(std::size_t total, std::size_t begin, std::size_t end) noexcept {
    auto comp_counter = 0ul;
    auto res_counter = 0ul;
    for (std::size_t i = begin; i < end; ++i) {
      for (std::size_t j = 1 + i; j < total; ++j) {
        ++comp_counter;
        if (collision_check(i, j)) {
          collision_resolve(i, j);
          ++res_counter;
        }
      }
    }
    comparisons += comp_counter;
    resolutions += res_counter;
  }

  void collision_quadtree(const std::vector<std::vector<std::int32_t>> &tree_container, std::size_t begin, std::size_t end) noexcept {
    auto comp_counter = 0ul;
    auto res_counter = 0ul;
    for (std::size_t k = begin; k < end; ++k) {
      for (std::size_t i = 0; i < tree_container[k].size(); ++i) {
        for (std::size_t j = i + 1; j < tree_container[k].size(); ++j) {
          ++comp_counter;
          if (collision_check(tree_container[k][i], tree_container[k][j])) {
            collision_resolve(tree_container[k][i], tree_container[k][j]);
            ++res_counter;
          }
        }
      }
    }
    comparisons += comp_counter;
    resolutions += res_counter;
  }

  void add_particle(const Particle& p, const glm::vec4& col) noexcept {
    ++particles_count;
    id.emplace_back(id.size());
    position.emplace_back(p.pos);
    velocity.emplace_back(p.vel);
    radii.emplace_back(p.radius);
    mass.emplace_back(p.mass);
    color.emplace_back(col);
  }

  void reserve(std::size_t size) noexcept {
    id.reserve(size);
    position.reserve(size);
    velocity.reserve(size);
    radii.reserve(size);
    mass.reserve(size);
    color.reserve(size);
  }

  void resize(std::size_t size) noexcept {
    id.resize(size);
    position.resize(size);
    velocity.resize(size);
    radii.resize(size);
    mass.resize(size);
    color.resize(size);
  }

  void clear() noexcept {
    particles_count = 0;
    id.clear();
    position.clear();
    velocity.clear();
    radii.clear();
    mass.clear();
    color.clear();
  }
};

//extern ParticleSystem particle_system;

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
  void refresh_vertices() noexcept;
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
