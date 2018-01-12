#include "athi_particle.h"
#include "athi_camera.h"
#include "athi_quadtree.h"
#include "athi_settings.h"
#include "athi_transform.h"
#include "athi_utility.h"
#include "athi_voxelgrid.h"
#include "athi_dispatch.h"
#include "athi_typedefs.h"

#include <array>
#include <future>

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

#define GLEW_STATIC
#include <GL/glew.h>

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#endif

ParticleManager particle_manager;

void ParticleManager::init() noexcept
{

  console->info("Particle object size: {} bytes", sizeof(Particle));
  console->info("Particles per cacheline(64 bytes): {} particles", 64 / sizeof(Particle));

  // OpenCL init
  //
  read_file("../Resources/particle_collision.cl", &kernel_source);
  if (!kernel_source)
    console->error("OpenCL missing kernel source");

  // Connect to a compute device
  err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1,
                       &device_id, NULL);
  if (err != CL_SUCCESS)
    console->error("Failed to create a device group!");

  // Create a compute context
  context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
  if (!context)
    console->error("Failed to create a compute context!");

  // Create a command commands
  commands = clCreateCommandQueue(context, device_id, 0, &err);
  if (!commands)
    console->error("Failed to create a command commands!");

  // Create the compute program from the source buffer
  program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source,
                                      NULL, &err);
  if (!program)
    console->error("Failed to create compute program!");

  // Build the program executable
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS)
  {
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

  ///////////////////////////

  // Shaders
  shader_program = glCreateProgram();
  const u32 vs =
      createShader("../Resources/athi_particle_shader.vs", GL_VERTEX_SHADER);
  const u32 fs =
      createShader("../Resources/athi_particle_shader.fs", GL_FRAGMENT_SHADER);

  glAttachShader(shader_program, vs);
  glAttachShader(shader_program, fs);

  glBindAttribLocation(shader_program, 0, "position");
  glBindAttribLocation(shader_program, 1, "color");
  glBindAttribLocation(shader_program, 2, "transform");

  glLinkProgram(shader_program);
  glValidateProgram(shader_program);
  validateShaderProgram("ParticleManager_init", shader_program);

  glDetachShader(shader_program, vs);
  glDetachShader(shader_program, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);

  // Setup the circle vertices
  std::vector<glm::vec2> positions;
  positions.reserve(num_verts);
  for (u32 i = 0; i < num_verts; ++i)
  {
    positions.emplace_back(cos(i * PI * 2.0f / num_verts),
                           sin(i * PI * 2.0f / num_verts));
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
  for (u32 i = 0; i < 4; ++i)
  {
    glEnableVertexAttribArray(2 + i);
    glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                          (void *)(i * sizeof(glm::vec4)));
    glVertexAttribDivisor(2 + i, 1);
  }
}

void ParticleManager::update_collisions() noexcept
{
  comparisons = 0;
  resolutions = 0;

  std::vector<std::vector<s32>> cont; // nodes with vec of particle.id's
  {
    profile p("ParticleManager::Quadtree/Voxelgrid input and get");

    if (quadtree_active && openCL_active == false)
    {
      quadtree = Quadtree<Particle>(glm::vec2(0.0f, 0.0f),
                                    glm::vec2(screen_width, screen_height));
      quadtree.input(particles);
      quadtree.get(cont);
    }
    else if (voxelgrid_active && openCL_active == false)
    {
      voxelgrid.reset();
      voxelgrid.input(particles);
      voxelgrid.get(cont);
    }
  }

#if 0
    // Search for duplicates (DEBUG)
    {
    s32 counter = 0;
    for (const auto& obj: cont) {
      counter += obj.size();
    }
    s32 duplicates = counter - static_cast<s32>(particles.size());
    if (duplicates != 0)
      console->warn("Duplicates found in container. num: {}", duplicates);
    }
#endif

  // Quadtree or Voxelgrid
  // Quadtree is significantly faster.
  if ((quadtree_active || voxelgrid_active) && openCL_active == false)
  {
    if (use_multithreading && variable_thread_count != 0)
    {

      const size_t thread_count = variable_thread_count;
      const size_t total = cont.size();
      const size_t parts = total / thread_count;
      const size_t leftovers = total % thread_count;

      if (use_libdispatch)
      {
        dispatch_apply(thread_count,
                       dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0),
                       ^(size_t i) {
                         const size_t begin = parts * i;
                         size_t end = parts * (i + 1);
                         if (i == thread_count - 1)
                           end += leftovers;
                         collision_quadtree(cont, begin, end);
                       });
      }
      else
      {
        std::vector<std::future<void>> results(thread_count);
        for (size_t i = 0; i < thread_count; ++i)
        {
          const size_t begin = parts * i;
          size_t end = parts * (i + 1);
          if (i == thread_count - 1)
            end += leftovers;
          results[i] = pool.enqueue(&ParticleManager::collision_quadtree, this, cont, begin, end);
        }

        for (auto &&res : results)
          res.get();
      }
    }
    else
      collision_quadtree(cont, 0, cont.size());
  }
  else if (use_multithreading && variable_thread_count != 0 &&
           openCL_active == false)
  {
    const size_t thread_count = variable_thread_count;
    const size_t total = particles.size();
    const size_t parts = total / thread_count;
    const size_t leftovers = total % thread_count;

    if (use_libdispatch)
    {
      dispatch_apply(thread_count,
                     dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0),
                     ^(size_t i) {
                       const size_t begin = parts * i;
                       size_t end = parts * (i + 1);
                       if (i == thread_count - 1)
                         end += leftovers;
                       collision_logNxN(total, begin, end);
                     });
    }
    else
    {
      std::vector<std::future<void>> results(thread_count);
      for (size_t i = 0; i < thread_count; ++i)
      {
        const size_t begin = parts * i;
        size_t end = parts * (i + 1);
        if (i == thread_count - 1)
          end += leftovers;
        results[i] = pool.enqueue(&ParticleManager::collision_logNxN, this, total, begin, end);
      }

      for (auto &&res : results)
        res.get();
    }
  }

  else if (openCL_active)
  {
    const u32 count = static_cast<u32>(particles.size());

    results.clear();
    results.resize(count);

    // Create the input and output arrays in device memory
    // for our calculation
    //
    input = clCreateBuffer(context, CL_MEM_READ_ONLY,
                           sizeof(Particle) * count, NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                            sizeof(Particle) * count, NULL, NULL);
    if (!input || !output)
    {
      console->error("Failed to allocate device memory!");
      exit(1);
    }

    // Write our data set s32o the input array in device
    // memory
    err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0,
                               sizeof(Particle) * count, &particles[0], 0,
                               NULL, NULL);
    if (err != CL_SUCCESS)
      console->error("Failed to write to source array!");

    // Set the arguments to our compute kernel
    err = 0;
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, 2, sizeof(u32), &count);
    if (err != CL_SUCCESS)
    {
      console->error("[line {}] Failed to set kernel arguments! {}", __LINE__, err);
      exit(1);
    }

    // Get the maximum work group size for executing the
    // kernel o dn the device
    //
    // err =
    //     clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE,
    //                              sizeof(size_t), &local, NULL);
    // if (err != CL_SUCCESS)
    // {
    //   console->error("[line {}] Failed to retrieve kernel work group info! {}", __LINE__, err);
    //   exit(1);
    // }

    global = count;
    //err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
    if (err)
    {
      console->error("[line {}] Failed to execute kernel! {}", __LINE__, err);
      exit(1);
    }

    // Wait for the command commands to get serviced before
    // reading back results
    clFinish(commands);

    // Read back the results from the device to verify the
    // output
    err = clEnqueueReadBuffer(commands, output, CL_TRUE, 0,
                              sizeof(Particle) * count, &results[0], 0, NULL,
                              NULL);
    if (err != CL_SUCCESS)
    {
      console->error("Failed to read output array! {}", err);
      exit(1);
    }

    particles = results;
  }

  // CPU Singlethreaded
  else
  {
    collision_logNxN(particles.size(), 0, particles.size());
  }
}

void ParticleManager::update() noexcept
{
  profile p("ParticleManager::update"); // @Todo: Make the profiler be a debug mode only tool.

  // just exit when no particles are present.
  if (particles.empty())
    return;

  // @TODO: This whole if statement and following logic statements. BE GONE.
  if (circle_collision)
  {
    profile p("ParticleManager::circle_collision");
    update_collisions();
  }
  {
    profile p("ParticleManager::update(draw nodes/color quadtree/voxelgrid)");
    if (quadtree_active && draw_debug)
    {
      if (color_particles)
        quadtree.color_objects(colors);
      if (draw_nodes)
        quadtree.draw_bounds();
    }
    if (voxelgrid_active && draw_debug)
    {
      if (color_particles)
        voxelgrid.color_objects(colors);
      if (draw_nodes)
        voxelgrid.draw_bounds();
    }
  }

  {
    profile p("ParticleManager::update(particles update)");
    for (auto &p : particles)
    {
      p.update();
    }
  }
}

void ParticleManager::update_gpu_buffers() noexcept
{
  const auto particles_size = particles.size();

  // Check if buffers need resizing
  if (particles_size > models.size())
  {
    models.resize(particles_size);
    transforms.resize(particles_size);
  }

  const auto proj = camera.get_ortho_projection();
  {
    // THIS IS THE SLOWEST THING EVER.
    profile p("ParticleManager::update(update buffers with new data)");
    // Update the buffers with the new data.

    for (const auto &p : particles)
    {
      // Update the transform
      transforms[p.id].pos = glm::vec3(p.pos.x, p.pos.y, 0);
      models[p.id] = proj * transforms[p.id].get_model();
    }
  }

  {
    profile p("ParticleManager::update(GPU buffer update)");
    // Update the gpu buffers incase of more particles..
    glBindBuffer(GL_ARRAY_BUFFER, vbo[TRANSFORM]);
    const size_t transform_bytes_needed = sizeof(glm::mat4) * particles_size;
    if (transform_bytes_needed > model_bytes_allocated)
    {
      glBufferData(GL_ARRAY_BUFFER, transform_bytes_needed, &models[0],
                   GL_STREAM_DRAW);
      model_bytes_allocated = transform_bytes_needed;
    }
    else
    {
      glBufferSubData(GL_ARRAY_BUFFER, 0, model_bytes_allocated, &models[0]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo[COLOR]);
    const size_t color_bytes_needed = sizeof(glm::vec4) * particles_size;
    if (color_bytes_needed > color_bytes_allocated)
    {
      glBufferData(GL_ARRAY_BUFFER, color_bytes_needed, &colors[0],
                   GL_STREAM_DRAW);
      color_bytes_allocated = color_bytes_needed;
    }
    else
    {
      glBufferSubData(GL_ARRAY_BUFFER, 0, color_bytes_allocated, &colors[0]);
    }
  }
}

void ParticleManager::draw() const noexcept
{
  glBindVertexArray(vao);
  glUseProgram(shader_program);
  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_verts,
                        static_cast<s32>(particles.size()));
}

void ParticleManager::add(const glm::vec2 &pos, f32 radius,
                          const glm::vec4 &color) noexcept
{
  Particle p;
  p.pos = pos;
  p.radius = radius;
  p.mass = 1.33333f * PI * radius * radius * radius;
  p.id = static_cast<s32>(particles.size());
  particles.emplace_back(p);

  Transform t;
  t.pos = glm::vec3(pos.x, pos.y, 0);
  t.scale = glm::vec3(radius, radius, 0);
  transforms.emplace_back(t);

  colors.emplace_back(color);
}

void ParticleManager::erase_all() noexcept
{
  particles.clear();
  colors.clear();
  transforms.clear();
}

bool ParticleManager::collision_check(const Particle &a,
                                      const Particle &b) const noexcept
{
  const f32 ax = a.pos.x;
  const f32 ay = a.pos.y;
  const f32 bx = b.pos.x;
  const f32 by = b.pos.y;
  const f32 ar = a.radius;
  const f32 br = b.radius;

  // square collision check
  if (ax - ar < bx + br && ax + ar > bx - br && ay - ar < by + br &&
      ay + ar > by - br)
  {
    const f32 dx = bx - ax;
    const f32 dy = by - ay;

    const f32 sum_radius = ar + br;
    const f32 sqr_radius = sum_radius * sum_radius;

    const f32 distance_sqrd = (dx * dx) + (dy * dy);

    // circle collision check
    if (distance_sqrd < sqr_radius)
      return true;
  }
  return false;
}

// Collisions response between two circles with varying radius and mass.
void ParticleManager::collision_resolve(Particle &a, Particle &b) noexcept
{
  // Local variables
  const f64 dx = b.pos.x - a.pos.x;
  const f64 dy = b.pos.y - a.pos.y;
  const f64 vdx = b.vel.x - a.vel.x;
  const f64 vdy = b.vel.y - a.vel.y;
  const glm::vec2 a_vel = a.vel;
  const glm::vec2 b_vel = b.vel;
  const f32 m1 = a.mass;
  const f32 m2 = b.mass;

  // Should the circles intersect. Seperate them. If not the next
  // calculated values will be off.
  separate(a, b);

  // A negative 'd' means the circles velocities are in opposite
  // directions
  const f64 d = dx * vdx + dy * vdy;

  // And we don't resolve collisions between circles moving away from
  // eachother
  if (d < 1e-4)
  {
    const glm::vec2 norm = glm::normalize(glm::vec2(dx, dy));
    const glm::vec2 tang = glm::vec2(norm.y * -1.0, norm.x);
    const f32 scal_norm_1 = glm::dot(norm, a_vel);
    const f32 scal_norm_2 = glm::dot(norm, b_vel);
    const f32 scal_tang_1 = glm::dot(tang, a_vel);
    const f32 scal_tang_2 = glm::dot(tang, b_vel);

    const f32 scal_norm_1_after =
        (scal_norm_1 * (m1 - m2) + 2.0f * m2 * scal_norm_2) / (m1 + m2);
    const f32 scal_norm_2_after =
        (scal_norm_2 * (m2 - m1) + 2.0f * m1 * scal_norm_1) / (m1 + m2);
    const glm::vec2 scal_norm_1_after_vec = norm * scal_norm_1_after;
    const glm::vec2 scal_norm_2_after_vec = norm * scal_norm_2_after;
    const glm::vec2 scal_norm_1_vec = tang * scal_tang_1;
    const glm::vec2 scal_norm_2_vec = tang * scal_tang_2;

    // Update velocities
    a.vel = (scal_norm_1_vec + scal_norm_1_after_vec) * 0.99f;
    b.vel = (scal_norm_2_vec + scal_norm_2_after_vec) * 0.99f;
  }
}

// Separates two s32ersecting circles.
void ParticleManager::separate(Particle &a, Particle &b) noexcept
{
  // Local variables
  const glm::vec2 a_pos = a.pos;
  const glm::vec2 b_pos = b.pos;
  const f32 ar = a.radius;
  const f32 br = b.radius;

  const f32 collision_depth = (ar + br) - glm::distance(b_pos, a_pos);

  const f32 dx = b_pos.x - a_pos.x;
  const f32 dy = b_pos.y - a_pos.y;

  // contact angle
  const f32 collision_angle = atan2(dy, dx);
  const f32 cos_angle = cos(collision_angle);
  const f32 sin_angle = sin(collision_angle);

  // TODO: could this be done using a normal vector and just inverting it?
  // amount to move each ball
  const f32 a_move_x = -collision_depth * 0.5f * cos_angle;
  const f32 a_move_y = -collision_depth * 0.5f * sin_angle;
  const f32 b_move_x = collision_depth * 0.5f * cos_angle;
  const f32 b_move_y = collision_depth * 0.5f * sin_angle;

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

// (N-1)*N/2
void ParticleManager::collision_logNxN(size_t total, size_t begin, size_t end) noexcept
{
  std::uint64_t comp_counter = 0;
  std::uint64_t res_counter = 0;
  for (size_t i = begin; i < end; ++i)
  {
    for (size_t j = 1 + i; j < total; ++j)
    {
      ++comp_counter;
      if (collision_check(particles[i], particles[j]))
      {
        collision_resolve(particles[i], particles[j]);
        ++res_counter;
      }
    }
  }
  comparisons += comp_counter;
  resolutions += res_counter;
}

void ParticleManager::collision_quadtree(
    const std::vector<std::vector<s32>> &cont, size_t begin, size_t end) noexcept
{
  std::uint64_t comp_counter = 0;
  std::uint64_t res_counter = 0;
  for (size_t k = begin; k < end; ++k)
  {
    for (size_t i = 0; i < cont[k].size(); ++i)
    {
      for (size_t j = i + 1; j < cont[k].size(); ++j)
      {
        ++comp_counter;
        if (collision_check(particles[cont[k][i]], particles[cont[k][j]]))
        {
          collision_resolve(particles[cont[k][i]], particles[cont[k][j]]);
          ++res_counter;
        }
      }
    }
  }
  comparisons += comp_counter;
  resolutions += res_counter;
}
