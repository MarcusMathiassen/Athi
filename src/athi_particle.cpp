#include "athi_particle.h"
#include "athi_camera.h"
#include "athi_transform.h"
#include "athi_utility.h"
#include "athi_quadtree.h"
#include "athi_voxelgrid.h"
#include "athi_settings.h"

#include <dispatch/dispatch.h>
#include <glm/gtx/vector_angle.hpp>
#include <glm/glm.hpp>

#define GLEW_STATIC
#include <GL/glew.h>

ParticleManager particle_manager;

void ParticleManager::init() {

  // OpenCL init
  //
  read_file("../Resources/particle_collision.cl", &kernel_source);
  if (!kernel_source)
    console->error("Error: OpenCL missing kernel source");
    

  // Connect to a compute device
  err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
  if (err != CL_SUCCESS)
    console->error("Error: Failed to create a device group!");

  // Create a compute context
  context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
  if (!context) console->error("Error: Failed to create a compute context!");

  // Create a command commands
  commands = clCreateCommandQueue(context, device_id, 0, &err);
  if (!commands) console->error("Error: Failed to create a command commands!");

  // Create the compute program from the source buffer
  program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source,
                                      NULL, &err);
  if (!program) console->error("Error: Failed to create compute program!");

  // Build the program executable
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];

    console->error("Error: Failed to build program executable!");
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG,
                          sizeof(buffer), buffer, &len);
    console->error(buffer);
  }

  // Create the compute kernel in the program we wish to run
  kernel = clCreateKernel(program, "particle_collision", &err);
  if (!kernel || err != CL_SUCCESS)
    console->error("Error: Failed to create compute kernel!");

  ///////////////////////////


  // Shaders
  shader_program = glCreateProgram();
  const uint32_t vs = createShader("../Resources/athi_circle_shader.vs", GL_VERTEX_SHADER);
  const uint32_t fs = createShader("../Resources/athi_circle_shader.fs", GL_FRAGMENT_SHADER);

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

  std::vector<glm::vec2> positions;
  positions.reserve(num_verts);
  for (uint32_t i = 0; i < num_verts; ++i) {
    positions.emplace_back(cos(i * M_PI * 2.0f / num_verts),
                           sin(i * M_PI * 2.0f / num_verts));
  }

  // vao
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // vbo
  glGenBuffers(NUM_BUFFERS, vbo);

  // POSITION
  glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION]);
  glBufferData(GL_ARRAY_BUFFER, num_verts * sizeof(positions[0]), &positions[0], GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  // COLOR
  glBindBuffer(GL_ARRAY_BUFFER, vbo[COLOR]);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glVertexAttribDivisor(1, 1);

  // TRANSFORM
  glBindBuffer(GL_ARRAY_BUFFER, vbo[TRANSFORM]);
  for (uint32_t i = 0; i < 4; ++i) {
    glEnableVertexAttribArray(2 + i);
    glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
    glVertexAttribDivisor(2 + i, 1);
  }
}

void ParticleManager::update() {

  // just exit when no particles are present.
  if (particles.empty()) return;

  if (circle_collision) {

    comparisons = 0;
    resolutions = 0;

    std::vector<std::vector<int>> cont; // nodes with vec of particle.id's 

    if (quadtree_active && openCL_active == false) {
      quadtree = Quadtree<Particle>(glm::vec2(0.0f, 0.0f), glm::vec2(screen_width, screen_height));
      quadtree.input(particles);
      quadtree.get(cont);
    } else if (voxelgrid_active && openCL_active == false) {
      voxelgrid.reset();
      voxelgrid.input(particles);
      voxelgrid.get(cont);
    }
    // Quadtree or Voxelgrid
    if ((quadtree_active || voxelgrid_active) && openCL_active == false) {
      if (use_multithreading && variable_thread_count != 0) {
        const size_t thread_count = variable_thread_count;
        const size_t total = cont.size();
        const size_t parts = total / thread_count;
        const size_t leftovers = total % thread_count;

        dispatch_apply(
          thread_count,
          dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0),
          ^(size_t i) {
          const size_t begin = parts * i;
          size_t end = parts * (i + 1);
          if (i == thread_count-1) end += leftovers;
          collision_quadtree(cont, begin, end);
        });
      } else
        collision_quadtree(cont, 0, cont.size());
    } else if (use_multithreading && variable_thread_count != 0 &&
               openCL_active == false) {
      const size_t thread_count = variable_thread_count;
      const size_t total = particles.size();
      const size_t parts = total / thread_count;
      const size_t leftovers = total % thread_count;

      dispatch_apply(
        thread_count,
        dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0),
        ^(size_t i) {
          const size_t begin = parts * i;
          size_t end = parts * (i + 1);
          if (i == thread_count-1) end += leftovers;
          collision_logNxN(total, begin, end);
        }
      );
    }

    else if (openCL_active) {

      const unsigned int count = particles.size();

      results.clear();
      results.resize(count);

      // Create the input and output arrays in device memory
      // for our calculation
      //
      input  = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Particle) * count, NULL, NULL);
      output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(Particle) * count, NULL, NULL);
      if (!input || !output) {
        console->error("Error: Failed to allocate device memory!");
        exit(1);
      }

      // Write our data set into the input array in device
      // memory
      err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(Particle) * count, &particles[0], 0, NULL, NULL);
      if (err != CL_SUCCESS)
        console->error("Error: Failed to write to source array!");

      // Set the arguments to our compute kernel
      err = 0;
      err =  clSetKernelArg(kernel, 0, sizeof(cl_mem),       &input);
      err |= clSetKernelArg(kernel, 1, sizeof(cl_mem),       &output);
      err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
      if (err != CL_SUCCESS) {
        console->error("Error: Failed to set kernel arguments! {}", err);
        exit(1);
      }

      // Get the maximum work group size for executing the
      // kernel o dn the device
      //
      err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
      if (err != CL_SUCCESS) {
        console->error("Error: Failed to retrieve kernel work group info! {}", err);
        exit(1);
      }

      global = count;
      // err = clEnqueueNDRangeKernel(commands, kernel, 1,
      // NULL, &global, &local, 0, NULL, NULL);
      err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, NULL, 0,
                                   NULL, NULL);
      if (err) {
        console->error("Error: Failed to execute kernel! {}", err);
        exit(1);
      }

      // Wait for the command commands to get serviced before
      // reading back results
      clFinish(commands);

      // Read back the results from the device to verify the
      // output
      err = clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(Particle) * count,
                                &results[0], 0, NULL, NULL);
      if (err != CL_SUCCESS) {
        console->error("Error: Failed to read output array! {}", err);
        exit(1);
      }

      particles = results;
    }

    // CPU Singlethreaded
    else {
      collision_logNxN(particles.size(), 0, particles.size());
    }
  }

  if (quadtree_active && draw_debug) {
    if (color_particles) quadtree.color_objects(colors);
    if (draw_nodes) quadtree.draw_bounds();
  }
  if (voxelgrid_active && draw_debug) {
    if (color_particles) voxelgrid.color_objects(colors);
    if (draw_nodes) voxelgrid.draw_bounds();
  }  

  for (auto &p : particles) {

    p.update();

    // Update the transform
    transforms[p.id].pos = glm::vec3(p.pos.x, p.pos.y, 0);
  }

  const size_t particles_size = particles.size();

  // Check if buffers need resizing
  if (particles_size > models.size()) {
    models.resize(particles_size);
  }

  const auto proj = camera.get_ortho_projection();

  // Update the buffers with the new data.
  for (const auto &p : particles) {
    models[p.id] = proj * transforms[p.id].get_model();
  }

  // Update the shader buffers incase of more particles..
  glBindBuffer(GL_ARRAY_BUFFER, vbo[TRANSFORM]);
  const size_t transform_bytes_needed = sizeof(glm::mat4) * particles_size;
  if (transform_bytes_needed > model_bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, transform_bytes_needed, &models[0], GL_STREAM_DRAW);
    model_bytes_allocated = transform_bytes_needed;
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_bytes_allocated, &models[0]);
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo[COLOR]);
  const size_t color_bytes_needed = sizeof(glm::vec4) * particles_size;
  if (color_bytes_needed > color_bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, color_bytes_needed, &colors[0], GL_STREAM_DRAW);
    color_bytes_allocated = color_bytes_needed;
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, color_bytes_allocated, &colors[0]);
  }
  
}

void ParticleManager::draw() const {
  glBindVertexArray(vao);
  glUseProgram(shader_program);
  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, num_verts, particles.size());
}

void ParticleManager::add(const glm::vec2 &pos, float radius, const glm::vec4 &color) {
  Particle p;
  p.pos = pos;
  p.radius = radius;
  p.mass = 1.33333f * M_PI * radius * radius * radius;
  p.id = static_cast<int32_t>(particles.size());
  particles.emplace_back(p);

  Transform t;
  t.pos = glm::vec3(pos.x, pos.y, 0);
  t.scale = glm::vec3(radius, radius, 0);
  transforms.emplace_back(t);

  colors.emplace_back(color);
}

void ParticleManager::erase_all() {
  particles.clear();
  colors.clear();
  transforms.clear();
}

bool ParticleManager::collision_check(const Particle &a, const Particle &b) const {
  const float ax = a.pos.x;
  const float ay = a.pos.y;
  const float bx = b.pos.x;
  const float by = b.pos.y;
  const float ar = a.radius;
  const float br = b.radius;

  // square collision check
  if (ax - ar < bx + br && ax + ar > bx - br && ay - ar < by + br &&
      ay + ar > by - br) {

    const float dx = bx - ax;
    const float dy = by - ay;

    const float sum_radius = ar + br;
    const float sqr_radius = sum_radius * sum_radius;

    const float distance_sqrd = (dx * dx) + (dy * dy);

    // circle collision check
    if (distance_sqrd < sqr_radius) return true;
  }
  return false;
}

// Collisions response between two circles with varying radius and mass.
void ParticleManager::collision_resolve(Particle &a, Particle &b) {
  // Local variables
  const double dx = b.pos.x - a.pos.x;
  const double dy = b.pos.y - a.pos.y;
  const double vdx = b.vel.x - a.vel.x;
  const double vdy = b.vel.y - a.vel.y;
  const glm::vec2 a_vel = a.vel;
  const glm::vec2 b_vel = b.vel;
  const float m1 = a.mass;
  const float m2 = b.mass;

  // Should the circles intersect. Seperate them. If not the next
  // calculated values will be off.
  separate(a, b);

  // A negative 'd' means the circles velocities are in opposite
  // directions
  const double d = dx * vdx + dy * vdy;

  // And we don't resolve collisions between circles moving away from
  // eachother
  if (d < 0.0) {
    const glm::vec2 norm = glm::normalize(glm::vec2(dx, dy));
    const glm::vec2 tang = glm::vec2(norm.y * -1.0, norm.x);
    const float scal_norm_1 = glm::dot(norm, a_vel);
    const float scal_norm_2 = glm::dot(norm, b_vel);
    const float scal_tang_1 = glm::dot(tang, a_vel);
    const float scal_tang_2 = glm::dot(tang, b_vel);

    const float scal_norm_1_after =
        (scal_norm_1 * (m1 - m2) + 2.0f * m2 * scal_norm_2) / (m1 + m2);
    const float scal_norm_2_after =
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

// Separates two intersecting circles.
void ParticleManager::separate(Particle &a, Particle &b) {
  // Local variables
  const glm::vec2 a_pos = a.pos;
  const glm::vec2 b_pos = b.pos;
  const float ar = a.radius;
  const float br = b.radius;

  const float collision_depth = (ar + br) - glm::distance(b_pos, a_pos);

  const float dx = b_pos.x - a_pos.x;
  const float dy = b_pos.y - a_pos.y;

  // contact angle
  const float collision_angle = atan2(dy, dx);
  const float cos_angle = cos(collision_angle);
  const float sin_angle = sin(collision_angle);

  // TODO: could this be done using a normal vector and just inverting it?
  // amount to move each ball
  const float a_move_x = -collision_depth * 0.5f * cos_angle;
  const float a_move_y = -collision_depth * 0.5f * sin_angle;
  const float b_move_x = collision_depth * 0.5f * cos_angle;
  const float b_move_y = collision_depth * 0.5f * sin_angle;

  // store the new move values
  glm::vec2 a_pos_move;
  glm::vec2 b_pos_move;

  // Make sure they dont moved beyond the border
  if (a_pos.x + a_move_x >= 0.0f + ar && a_pos.x + a_move_x <= screen_width - ar)
    a_pos_move.x += a_move_x;
  if (a_pos.y + a_move_y >= 0.0f + ar && a_pos.y + a_move_y <= screen_height - ar)
    a_pos_move.y += a_move_y;
  if (b_pos.x + b_move_x >= 0.0f + br && b_pos.x + b_move_x <= screen_width - br)
    b_pos_move.x += b_move_x;
  if (b_pos.y + b_move_y >= 0.0f + br && b_pos.y + b_move_y <= screen_height - br)
    b_pos_move.y += b_move_y;

  // Update positions
  a.pos += a_pos_move;
  b.pos += b_pos_move;
}

// (N-1)*N/2
void ParticleManager::collision_logNxN(size_t total, size_t begin, size_t end) {
  uint64_t comp_counter = 0;
  uint64_t res_counter = 0;
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

void ParticleManager::collision_quadtree(const std::vector<std::vector<int>> &cont, size_t begin, size_t end) {
  uint64_t comp_counter = 0;
  uint64_t res_counter = 0;
  for (size_t k = begin; k < end; ++k) {
    for (size_t i = 0; i < cont[k].size(); ++i) {
      for (size_t j = i + 1; j < cont[k].size(); ++j) {
        ++comp_counter;
        if (collision_check(particles[cont[k][i]], particles[cont[k][j]])) {
          collision_resolve(particles[cont[k][i]], particles[cont[k][j]]);
          ++res_counter;
        }
      }
    }
  }
  comparisons += comp_counter;  
  resolutions += res_counter;  
}
