#include "athi_particle.h"
#include "athi_camera.h"
#include "athi_dispatch.h"
#include "athi_quadtree.h"
#include "athi_settings.h"
#include "athi_transform.h"
#include "athi_typedefs.h"
#include "athi_utility.h"
#include "athi_voxelgrid.h"

#include <algorithm>
#include <array>
#include <future>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

#define GLEW_STATIC
#include <GL/glew.h>

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#endif

ParticleManager particle_manager;

void ParticleManager::opencl_init() noexcept {
  // Read in the kernel source
  read_file("../Resources/particle_collision.cl", &kernel_source);
  if (!kernel_source) console->error("OpenCL missing kernel source");

  // Connect to a compute device
  err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1,
                       &device_id, NULL);
  if (err != CL_SUCCESS) console->error("Failed to create a device group!");

  // Create a compute context
  context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
  if (!context) console->error("Failed to create a compute context!");

  // Create a command commands
  commands = clCreateCommandQueue(context, device_id, 0, &err);
  if (!commands) console->error("Failed to create a command commands!");

  // Create the compute program from the source buffer
  program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source,
                                      NULL, &err);
  if (!program) console->error("Failed to create compute program!");

  // Build the program executable
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];

    console->error("Failed to build program executable!");
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
                          sizeof(buffer), buffer, &len);
    console->error(buffer);
  }

  // Create the compute kernel in the program we wish to run
  kernel = clCreateKernel(program, "particle_collision", &err);
  if (!kernel || err != CL_SUCCESS)
    console->error("Failed to create compute kernel!");
}

void ParticleManager::init() noexcept {
  // Print some debug info about particle sizes
  console->info("Particle object size: {} bytes", sizeof(Particle));
  console->info("Particles per cacheline(64 bytes): {} particles",
                64 / sizeof(Particle));

  // OpenCL
  opencl_init();

  // Shaders
  shader.init("ParticleManager::init()");
  shader.load_from_file("../Resources/default_particle_shader.vert",
                        ShaderType::Vertex);
  shader.load_from_file("../Resources/default_particle_shader.frag",
                        ShaderType::Fragment);
  shader.bind_attrib("position");
  shader.bind_attrib("color");
  shader.bind_attrib("transform");
  shader.link();

  // Setup the circle vertices
  std::vector<glm::vec2> positions;
  positions.reserve(num_verts);
  for (auto i = 0; i < num_verts; ++i) {
    positions.emplace_back(cosf(i * PI * 2.0f / num_verts),
                           sinf(i * PI * 2.0f / num_verts));
  }

  // VAO
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // VBO
  glGenBuffers(NUM_BUFFERS, vbo);

  // POSITION
  glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION]);
  glBufferData(GL_ARRAY_BUFFER, num_verts * sizeof(positions[0]), &positions[0],
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  // COLOR
  glBindBuffer(GL_ARRAY_BUFFER, vbo[COLOR]);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glVertexAttribDivisor(1, 1);

  // TRANSFORM
  glBindBuffer(GL_ARRAY_BUFFER, vbo[TRANSFORM]);
  for (u32 i = 0; i < 4; ++i) {
    glEnableVertexAttribArray(2 + i);
    glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                          (void *)(i * sizeof(glm::vec4)));
    glVertexAttribDivisor(2 + i, 1);
  }
}

inline static auto get_begin_and_end(int i, int total, int threads) noexcept {
  const int parts = total / threads;
  const int leftovers = total % threads;
  const int begin = parts * i;
  int end = parts * (i + 1);
  if (i == threads - 1) end += leftovers;
  return std::tuple<std::size_t, std::size_t>{begin, end};
};

void ParticleManager::opencl_naive() noexcept {
  const auto count = static_cast<std::uint32_t>(particles.size());

  // Create the input and output arrays in device memory
  // for our calculation
  //
  input = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Particle) * count,
                         NULL, NULL);
  output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(Particle) * count,
                          NULL, NULL);
  if (!input || !output) {
    console->error("Failed to allocate device memory!");
    exit(1);
  }

  // Write our data set s32o the input array in device
  // memory
  err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0,
                             sizeof(Particle) * count, &particles[0], 0, NULL,
                             NULL);

  if (err != CL_SUCCESS) console->error("Failed to write to source array!");

  // Set the arguments to our compute kernel
  err = 0;
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
  err |= clSetKernelArg(kernel, 2, sizeof(std::uint32_t), &count);
  // err |= clSetKernelArg(kernel, 3, sizeof(cl_mem) * local, NULL);
  if (err != CL_SUCCESS) {
    console->error("[line {}] Failed to set kernel arguments! {}", __LINE__,
                   err);
    exit(1);
  }

  // Get the maximum work group size for executing the
  // kernel on the device
  err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE,
                                 sizeof(local), &local, NULL);
  if (err != CL_SUCCESS) {
    console->error("[line {}] Failed to retrieve kernel work group info!{}",
                   __LINE__, err);
    exit(1);
  }

  const auto leftovers = count % local;
  global_dim = count - leftovers;  // 1D
  console->info("OpenCL count: {}", count);
  console->info("OpenCL local: {}", local);
  console->info("OpenCL global_dim: {}", global_dim);
  console->info("OpenCL leftovers run on CPU: {}", leftovers);

  err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global_dim, &local,
                               0, NULL, NULL);
  // err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global_dim, NULL,
  // 0, NULL, NULL);
  if (err) {
    console->error("[line {}] Failed to execute kernel! {}", __LINE__, err);
    exit(1);
  }

  // Wait for the command commands to get serviced before
  // reading back results
  clFinish(commands);

  // Read back the results from the device to verify the
  // output
  err = clEnqueueReadBuffer(commands, output, CL_TRUE, 0,
                            sizeof(Particle) * count, &particles[0], 0, NULL,
                            NULL);
  if (err != CL_SUCCESS) {
    console->error("Failed to read output array! {}", err);
    exit(1);
  }

  // Handle any leftover that werent checked
  if (leftovers) {
    const size_t total = leftovers;
    dispatch_apply(variable_thread_count,
                   dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0),
                   ^(size_t i) {
                     const auto[begin, end] =
                         get_begin_and_end(i, total, variable_thread_count);
                     collision_logNxN(total, count - leftovers + begin, end);
                   });
  }
}

void ParticleManager::update_collisions() noexcept {
  // reset the values
  comparisons = 0;
  resolutions = 0;

  // Use a tree to partition the data
  std::vector<std::vector<std::int32_t>> tree_container;
  switch (tree_type) {
    using Tree = TreeType;
    case Tree::Quadtree: {
      quadtree =
          Quadtree<Particle>({0.0f, 0.0f}, {screen_width, screen_height});
      {
        profile p("Quadtree.input()");
        quadtree.input(particles);
      }
      {
        profile p("Quadtree.get()");
        quadtree.get(tree_container);
      }
      break;
    }
    case Tree::UniformGrid: {
      {
        profile p("voxelgrid.reset()");
        voxelgrid.reset();
      }
      {
        profile p("voxelgrid.input()");
        voxelgrid.input(particles);
      }
      {
        profile p("voxelgrid.get()");
        voxelgrid.get(tree_container);
      }
      break;
    }
    default: /* Using naive approch */
      break;
  }

  if (openCL_active && particles.size() >= 256) {
    opencl_naive();
    return;
  }

  if (use_multithreading && variable_thread_count != 0) {
    if constexpr (os == OS::Apple) {
      threadpool_solution = ThreadPoolSolution::AppleGCD;
    } else {
      threadpool_solution = ThreadPoolSolution::Dispatch;
    }
  } else
    threadpool_solution = ThreadPoolSolution::None;

  profile p("ParticleManager::update(circle_collision");

  const std::size_t total = particles.size();
  const std::size_t container_total = tree_container.size();

  switch (threadpool_solution) {
    using Threads = ThreadPoolSolution;
    case Threads::AppleGCD: {
      switch (tree_type) {
        using Tree = TreeType;
        case Tree::Quadtree:
          [[fallthrough]];
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
        case Tree::Quadtree:
          [[fallthrough]];
        case Tree::UniformGrid: {
          std::vector<std::future<void>> results(variable_thread_count);
          for (int i = 0; i < variable_thread_count; ++i) {
            const auto[begin, end] =
                get_begin_and_end(i, container_total, variable_thread_count);
            results[i] = pool.enqueue(&ParticleManager::collision_quadtree,
                                      this, tree_container, begin, end);
          }
          for (auto &&res : results) res.get();
        } break;

        case Tree::None: {
          std::vector<std::future<void>> results(variable_thread_count);
          for (int i = 0; i < variable_thread_count; ++i) {
            const auto[begin, end] =
                get_begin_and_end(i, total, variable_thread_count);
            results[i] = pool.enqueue(&ParticleManager::collision_logNxN, this,
                                      total, begin, end);
          }
          for (auto &&res : results) res.get();
        } break;
      }
    } break;

    case Threads::None: {
      switch (tree_type) {
        using Tree = TreeType;
        case Tree::Quadtree:
          [[fallthrough]];
        case Tree::UniformGrid: {
          collision_quadtree(tree_container, 0, container_total);
        } break;
        case Tree::None: {
          collision_logNxN(total, 0, total);
        } break;
      }
    } break;
  }
}

void ParticleManager::draw_debug_nodes() noexcept {
  if (particles.empty()) return;
  profile p("ParticleManager::draw_debug_nodes");

  if (draw_debug) {
    switch (tree_type) {
      using TT = TreeType;
      case TT::Quadtree: {
        if (color_particles) quadtree.color_objects(colors);
        if (draw_nodes) quadtree.draw_bounds();
      } break;

      case TT::UniformGrid: {
        if (color_particles) voxelgrid.color_objects(colors);
        if (draw_nodes) voxelgrid.draw_bounds();
      } break;

      case TT::None: {
      } break;
    }
  }
}

void ParticleManager::update() noexcept {
  // @TODO: This whole if statement and following logic statements. BE GONE.
  {
    profile p("update_collisions() + particles.update()");
   for (int i = 0; i < physics_samples; ++i) {
     const auto start = glfwGetTime();
     if (circle_collision) {
       update_collisions();
     }

     {
       profile p("ParticleManager::update(particles::update)");
       for (auto &p : particles) {
         p.update();
       }
     }
     timestep = (((glfwGetTime() - start) * 1000.0) / (1000.0 / 60.0)) / physics_samples;
   }
  }

  if (use_gravitational_force) {
    profile p("ParticleManager::apply_n_body()");
    apply_n_body();
  }
}

void ParticleManager::update_gpu_buffers() noexcept {
  if (particles.empty()) return;
  profile p("ParticleManager::update_gpu_buffers");

  const auto particles_size = particles.size();

  // Check if buffers need resizing
  if (particles_size > models.size()) {
    models.resize(particles_size);
    transforms.resize(particles_size);
  }

  const auto proj = camera.get_ortho_projection();
  {
    // THIS IS THE SLOWEST THING EVER.
    profile p(
        "ParticleManager::update_gpu_buffers(update buffers with new data)");
    // Update the buffers with the new data.
    for (const auto &p : particles) {
      // Update the transform
      transforms[p.id].pos = {p.pos.x, p.pos.y, 0.0f};
      models[p.id] = proj * transforms[p.id].get_model();
    }
  }

  {
    profile p("ParticleManager::update_gpu_buffers(GPU buffer update)");
    // Update the gpu buffers incase of more particles..
    glBindBuffer(GL_ARRAY_BUFFER, vbo[TRANSFORM]);
    const auto transform_bytes_needed = sizeof(glm::mat4) * particles_size;
    if (transform_bytes_needed > model_bytes_allocated) {
      glBufferData(GL_ARRAY_BUFFER, transform_bytes_needed, &models[0],
                   GL_STREAM_DRAW);
      model_bytes_allocated = transform_bytes_needed;
    } else {
      glBufferSubData(GL_ARRAY_BUFFER, 0, model_bytes_allocated, &models[0]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo[COLOR]);
    const auto color_bytes_needed = sizeof(glm::vec4) * particles_size;
    if (color_bytes_needed > color_bytes_allocated) {
      glBufferData(GL_ARRAY_BUFFER, color_bytes_needed, &colors[0],
                   GL_STREAM_DRAW);
      color_bytes_allocated = color_bytes_needed;
    } else {
      glBufferSubData(GL_ARRAY_BUFFER, 0, color_bytes_allocated, &colors[0]);
    }
  }
}

void ParticleManager::draw() noexcept {
  if (particles.empty()) return;
  profile p("ParticleManager::draw");
  glBindVertexArray(vao);
  shader.bind();
  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_verts,
                        static_cast<std::int32_t>(particles.size()));
}

void ParticleManager::add(const glm::vec2 &pos, float radius,
                          const glm::vec4 &color) noexcept {
  Particle p;
  p.pos = pos;
  p.radius = radius;
  p.mass = 1.33333f * PI * radius * radius * radius;
  p.id = static_cast<std::int32_t>(particles.size());
  particles.emplace_back(p);

  Transform t;
  t.pos = {pos.x, pos.y, 0};
  t.scale = {radius, radius, 0};
  transforms.emplace_back(t);

  colors.emplace_back(color);
}

void ParticleManager::erase_all() noexcept {
  particles.clear();
  colors.clear();
  transforms.clear();
}

bool ParticleManager::collision_check(const Particle &a,
                                      const Particle &b) const noexcept {
  // Local variables
  const auto ax = a.pos.x;
  const auto ay = a.pos.y;
  const auto bx = b.pos.x;
  const auto by = b.pos.y;
  const auto ar = a.radius;
  const auto br = b.radius;

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

// Collisions response between two circles with varying radius and mass.
void ParticleManager::collision_resolve(Particle &a, Particle &b) const noexcept {
  // Local variables
  const auto dx = b.pos.x - a.pos.x;
  const auto dy = b.pos.y - a.pos.y;
  const auto vdx = b.vel.x - a.vel.x;
  const auto vdy = b.vel.y - a.vel.y;
  const auto a_vel = a.vel;
  const auto b_vel = b.vel;
  const auto m1 = a.mass;
  const auto m2 = b.mass;

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

    const auto scal_norm_1_after =
        (scal_norm_1 * (m1 - m2) + 2.0f * m2 * scal_norm_2) / (m1 + m2);
    const auto scal_norm_2_after =
        (scal_norm_2 * (m2 - m1) + 2.0f * m1 * scal_norm_1) / (m1 + m2);
    const auto scal_norm_1_after_vec = norm * scal_norm_1_after;
    const auto scal_norm_2_after_vec = norm * scal_norm_2_after;
    const auto scal_norm_1_vec = tang * scal_tang_1;
    const auto scal_norm_2_vec = tang * scal_tang_2;

    // Update velocities
    a.vel = (scal_norm_1_vec + scal_norm_1_after_vec) * collision_energy_loss;
    b.vel = (scal_norm_2_vec + scal_norm_2_after_vec) * collision_energy_loss;
  }
}

// Separates two s32ersecting circles.
void ParticleManager::separate(Particle &a, Particle &b) const noexcept {
  // Local variables
  const auto a_pos = a.pos;
  const auto b_pos = b.pos;
  const auto ar = a.radius;
  const auto br = b.radius;

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
  if (a_pos.x + a_move_x >= 0.0f + ar &&
      a_pos.x + a_move_x <= screen_width - ar)
    a_pos_move.x += a_move_x;
  if (a_pos.y + a_move_y >= 0.0f + ar &&
      a_pos.y + a_move_y <= screen_height - ar)
    a_pos_move.y += a_move_y;
  if (b_pos.x + b_move_x >= 0.0f + br &&
      b_pos.x + b_move_x <= screen_width - br)
    b_pos_move.x += b_move_x;
  if (b_pos.y + b_move_y >= 0.0f + br &&
      b_pos.y + b_move_y <= screen_height - br)
    b_pos_move.y += b_move_y;

  // Update positions
  a.pos += a_pos_move;
  b.pos += b_pos_move;
}


static void gravitational_force(Particle &a, const Particle &b) {
  const float x1 = a.pos.x;
  const float y1 = a.pos.y;
  const float x2 = b.pos.x;
  const float y2 = b.pos.y;
  const float m1 = a.mass;
  const float m2 = b.mass;

  const float dx = x2 - x1;
  const float dy = y2 - y1;
  const float d = sqrt(dx * dx + dy * dy);

  if (d > 1e-4f) {
    const float angle = atan2(dy, dx);
    const float G = gravitational_constant;
    const float F = G * m1 * m2 / d * d;

    a.vel.x += F * cos(angle);
    a.vel.y += F * sin(angle);
  }
}

void ParticleManager::apply_n_body() noexcept {
  for (size_t i = 0; i < particles.size(); ++i) {
    for (size_t j = 0; j < particles.size(); ++j) {
      gravitational_force(particles[i], particles[j]);
    }
  }
}

// (N-1)*N/2
void ParticleManager::collision_logNxN(size_t total, size_t begin,
                                       size_t end) noexcept {
  auto comp_counter = 0ul;
  auto res_counter = 0ul;
  for (size_t i = begin; i < end; ++i) {
    for (size_t j = 1 + i; j < total; ++j) {
      ++comp_counter;
      if (collision_check(particles[i], particles[j])) {
        collision_resolve(particles[i], particles[j]);
        ++res_counter;
      }
    }
  }
  comparisons += comp_counter;
  resolutions += res_counter;
}

void ParticleManager::collision_quadtree(
    const std::vector<std::vector<std::int32_t>> &tree_container, size_t begin,
    size_t end) noexcept {
  auto comp_counter = 0ul;
  auto res_counter = 0ul;
  for (size_t k = begin; k < end; ++k) {
    for (size_t i = 0; i < tree_container[k].size(); ++i) {
      for (size_t j = i + 1; j < tree_container[k].size(); ++j) {
        ++comp_counter;
        if (collision_check(particles[tree_container[k][i]],
                            particles[tree_container[k][j]])) {
          collision_resolve(particles[tree_container[k][i]],
                            particles[tree_container[k][j]]);
          ++res_counter;
        }
      }
    }
  }
  comparisons += comp_counter;
  resolutions += res_counter;
}
