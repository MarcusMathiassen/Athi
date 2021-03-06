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

#include "athi_particle.h"

#include "./Utility/athi_constant_globals.h" // kPI, kGravitationalConstant
#include "./Utility/athi_save_state.h" // write_data, read_data
#include "./Renderer/athi_camera.h" // Camera
#include "athi_settings.h"
#include "Utility/console.h" // console
#include "athi_utility.h" // read_file, get_begin_and_end

#include "athi_transform.h"  // Transform

#include <algorithm>  // std::min_element, std::max_element

ParticleSystem particle_system;

void ParticleSystem::buffered_call(std::function<void()>&& f) noexcept
{
  std::unique_lock<std::mutex> lock(buffered_call_mutex);
  buffered_call_buffer.emplace_back(std::move(f));
}

void ParticleSystem::execute_buffered_calls() noexcept
{
  if (buffered_call_buffer.empty()) return;

  std::unique_lock<std::mutex> lock(buffered_call_mutex);

  // Execute all stores callseq
  for (auto &c : buffered_call_buffer)
  {
    c();
  }

  // Clear the buffer for next frame
  buffered_call_buffer.clear();
}

void ParticleSystem::update_particles(int begin, int end, f32 dt) noexcept
{
  for (int i = begin; i < end; ++i)
  {
    position[i].x += velocity[i].x * dt * time_scale * air_resistance;
    position[i].y += velocity[i].y * dt * time_scale * air_resistance;

    // Border collision
    if (border_collision)
    {
      if (position[i].x < 0 + radius[i])
      {
        position[i].x = 0 + radius[i];
        velocity[i].x = -velocity[i].x * collision_energy_loss;
      }
      if (position[i].x > framebuffer_width - radius[i])
      {
        position[i].x = framebuffer_width - radius[i];
        velocity[i].x = -velocity[i].x * collision_energy_loss;
      }
      if (position[i].y < 0 + radius[i])
      {
        position[i].y = 0 + radius[i];
        velocity[i].y = -velocity[i].y * collision_energy_loss;
      }
      if (position[i].y > framebuffer_height - radius[i])
      {
        position[i].y = framebuffer_height - radius[i];
        velocity[i].y = -velocity[i].y * collision_energy_loss;
      }
    }
  }
}


void ParticleSystem::init() noexcept {
  // Print some debug info about particle sizes
  console->info("Particle object size: {} bytes", sizeof(Particle));
  console->info("Particles per cacheline(64 bytes): {} particles",
                64 / sizeof(Particle));

  // Loads in any saved state
  load_state();

  // OpenCL
  opencl_init();


  if constexpr (!use_textured_particles) {

      // Clear previous values
      vertices.resize(num_vertices_per_particle);
      indices.resize(num_vertices_per_particle * 3);

      // Add indices
      for (int n = 0; n < num_vertices_per_particle - 2; ++n)
      {
          indices[n] = 0;
          indices[n+1] = n + 1;
          indices[n+2] = n + 2;
      }

      // Add vertices
      for (int i = 0; i < num_vertices_per_particle; ++i)
      {
        const auto cont = i * kPI * 2 / num_vertices_per_particle;
        vertices[i] = {cos(cont), sin(cont)};
      }

    auto &shader = renderer.make_shader();
    shader.sources = {
      "default_particle_shader.vert",
      "default_particle_shader.frag"
    };
    shader.attribs = {"vertices", "position", "color", "radius"};
    shader.uniforms = {"viewport_size"};

    auto &vertex_buffer = renderer.make_buffer("vertices");
    vertex_buffer.data = &vertices[0];
    vertex_buffer.data_size = vertices.size() * sizeof(vertices[0]);
    vertex_buffer.data_members = 2;
    vertex_buffer.type = buffer_type::array_buffer;
    vertex_buffer.usage = buffer_usage::static_draw;

    auto &position_buffer = renderer.make_buffer("position");
    position_buffer.data_members = 2;
    position_buffer.type = buffer_type::array_buffer;
    position_buffer.usage = buffer_usage::dynamic_draw;
    position_buffer.divisor = 1;

    auto &color_buffer = renderer.make_buffer("color");
    color_buffer.data_members = 4;
    color_buffer.type = buffer_type::array_buffer;
    color_buffer.usage = buffer_usage::dynamic_draw;
    color_buffer.divisor = 1;

    auto &radius_buffer = renderer.make_buffer("radius");
    radius_buffer.data_members = 1;
    radius_buffer.type = buffer_type::array_buffer;
    radius_buffer.usage = buffer_usage::dynamic_draw;
    radius_buffer.divisor = 1;

    // auto &indices_buffer = renderer.make_buffer("indices");
    // indices_buffer.data = &indices[0];
    // indices_buffer.data_size = indices.size() * sizeof(indices[0]);
    // indices_buffer.type = buffer_type::element_array;

    renderer.finish();


  } else {

    tex = {particle_texture, GL_LINEAR};

    auto &shader = renderer.make_shader();
    shader.sources = {"billboard_particle_shader.vert",
                      "billboard_particle_shader.frag"};
    shader.attribs = {"radius", "color", "transform"};

    auto &radius = renderer.make_buffer("radius");
    radius.data_members = 1;
    radius.divisor = 1;

    auto &colors = renderer.make_buffer("colors");
    colors.data_members = 4;
    colors.divisor = 1;

    auto &transforms = renderer.make_buffer("transforms");
    transforms.data_members = 4;
    transforms.stride = sizeof(mat4);
    transforms.pointer = sizeof(vec4);
    transforms.divisor = 1;
    transforms.is_matrix = true;

    // GLushort indices[6]{0, 1, 2, 0, 2, 3};
    // auto &indices_buffer = renderer.make_buffer("indices");
    // indices_buffer.data = (void*)indices;
    // indices_buffer.data_size = sizeof(indices);
    // indices_buffer.type = buffer_type::element_array;

    renderer.finish();
  }
}

// Passes the commandbuffer to the renderer
// @Hot:  Called every frame.
// @GPU:  Uses the renderer.
void ParticleSystem::draw() noexcept
{
  if (particle_count == 0) return;

  if constexpr (!use_textured_particles) {
    CommandBuffer cmd_buffer;
    cmd_buffer.type = primitive::triangle_fan;
    cmd_buffer.count = num_vertices_per_particle;
    // cmd_buffer.has_indices = true;
    cmd_buffer.primitive_count = particle_count;

    renderer.bind();

    renderer.shader.set_uniform("viewport_size", glm::vec2(framebuffer_width, framebuffer_height));

    renderer.draw(cmd_buffer);
  } else {

    // Billboard
    CommandBuffer cmd_buffer;
    cmd_buffer.type = primitive::triangles;
    cmd_buffer.count = 6;
    // cmd_buffer.has_indices = true;
    cmd_buffer.primitive_count = particle_count;

    renderer.bind();

    tex.bind(0);

    renderer.draw(cmd_buffer);
  }
}

static vector<float> radii;

// @CPU
void ParticleSystem::update_data() noexcept
{
  // If there are any buffered calls, execute them
  execute_buffered_calls();

  if (particle_count == 0) return;

  if (is_particles_colored_by_acc)
  {
    // Update the buffers with the new data.
    if (multithreaded_particle_update && use_multithreading)
    {
      dispatch.parallel_for_each(position, [this](size_t begin, size_t end)
      {
       for (size_t i = begin; i < end; ++i)
          {
            auto &p = position[i];
            auto &v = velocity[i];

            const auto old = p - v;
            const auto pos_diff = p - old;
            color[i] = color_by_acceleration(acceleration_color_min,
                                             acceleration_color_max, pos_diff);
          }
        });
    }
    else
    {
      for (size_t i = 0; i < particle_count; ++i)
      {
        auto &p = position[i];
        auto &v = velocity[i];

        const auto old = p - v;
        const auto pos_diff = p - old;
        color[i] = color_by_acceleration(acceleration_color_min,
                                         acceleration_color_max, pos_diff);
      }
    }
  }
}

// @GPU
void ParticleSystem::gpu_buffer_update() noexcept
{
  // Update the gpu buffers incase of more particles..
  if (!position.empty())  { renderer.update_buffer("position",  position); }
  if (!color.empty())     { renderer.update_buffer("color",     color); }
  if (!radius.empty())    { renderer.update_buffer("radius",    radius); }
}

// @CPU
void ParticleSystem::rebuild_vertices(u32 num_vertices) noexcept
{
  num_vertices_per_particle = num_vertices;

  // We cant draw anything with less than 3 vertices so just return
  if (num_vertices < 3) { return; }

  // Clear previous values
  vertices.resize(num_vertices);
  indices.resize(num_vertices * 3);

  // Add indices
  for (int n = 0; n < num_vertices - 2; ++n)
  {
      indices[n] = 0;
      indices[n+1] = n + 1;
      indices[n+2] = n + 2;
  }

  // Add vertices
  for (int i = 0; i < num_vertices; ++i)
  {
    const auto cont = i * kPI * 2 / num_vertices;
    vertices[i] = {cos(cont), sin(cont)};
  }

  // Update the GPU buffers
  if constexpr (!use_textured_particles)
  {
    renderer.update_buffer("vertices", vertices);
    renderer.update_buffer("indices", indices);
  }
}

auto get_min_and_max_pos(const vector<glm::vec2>& position)
{
  glm::vec2 max = {static_cast<float>(-INT_MAX),  static_cast<float>(-INT_MAX)};
  glm::vec2 min = {static_cast<float>( INT_MAX),  static_cast<float>( INT_MAX)};
  for (const auto &p: position)
  {
    max.x = (p.x > max.x) ? p.x : max.x;
    max.y = (p.y > max.y) ? p.y : max.y;
    min.x = (p.x < min.x) ? p.x : min.x;
    min.y = (p.y < min.y) ? p.y : min.y;
  }
  return std::tuple<glm::vec2, glm::vec2>(min, max);
}

void ParticleSystem::update_collisions() noexcept
{
  // reset the values
  comparisons = 0;
  resolutions = 0;

  if (openCL_active && particle_count >= 256) {
    opencl_naive();
    return;
  }

  switch (tree_type)
  {
    using Tree = TreeType;
    case Tree::Quadtree: [[fallthrough]];
    case Tree::UniformGrid: {
      if (use_multithreading)
      {
        dispatch.parallel_for_each(tree_container, [this](size_t begin, size_t end)
        {
          collision_quadtree(tree_container, begin, end);
        });
      }
      else {
        collision_quadtree(tree_container, 0, tree_container.size());
      }
    } break;

    case Tree::None: {
      if (use_multithreading) {
        dispatch.parallel_for_each(position, [this](size_t begin, size_t end)
        {
           collision_logNxN(particle_count, begin, end);
        });
      }
      else {
          collision_logNxN(particle_count, 0, particle_count);
      }
    } break;
  }
}

void ParticleSystem::draw_debug_nodes() noexcept {
  if (particle_count == 0) return;


  if (draw_debug) {

    // draw the collision box
    for (int i = 0; i < particle_count; ++i) {
      draw_rect
      (
        position[i] - radius[i], // min
        position[i] + radius[i], // max
        debug_color,      // color
        true
      );
    }

    switch (tree_type) {
      case TreeType::Quadtree: {
        if (circle_collision && draw_nodes) quadtree.draw_bounds(quadtree_show_only_occupied, debug_color);
      } break;

      case TreeType::UniformGrid: {
        if (circle_collision && draw_nodes) uniformgrid.draw_bounds(debug_color);
      } break;

      case TreeType::None: {
      } break;
    }
  }
}

void ParticleSystem::update(float dt) noexcept
{
  // @Hack
  if constexpr (multithreaded_engine)
    std::unique_lock<std::mutex> lck(particles_mutex);

  if (particle_count == 0) return;


  if (!circle_collision) return;

  // Get the optimal bounds for our tree
  vec2 min, max;
  if (tree_optimized_size) {
    const auto[mi, ma] = get_min_and_max_pos(position);
    min = mi;
    max = ma;
  }

  // Use a tree to partition the data
  tree_container.clear();
  switch (tree_type) {
    using Tree = TreeType;
    case Tree::None: {} break;
    case Tree::Quadtree: {

      Quadtree::max_depth = quadtree_depth;
      Quadtree::max_capacity = quadtree_capacity;

      if (tree_optimized_size)
        quadtree = Quadtree(min, max);
      else
        quadtree = Quadtree({0.0f, 0.0f}, {framebuffer_width, framebuffer_height});

      {
        quadtree.set_data(position, radius);
        quadtree.input_range(0, particle_count);
        quadtree.get(tree_container);
      }
    } break;
    case Tree::UniformGrid: {
      // {

      //   if (tree_optimized_size)
      //     uniformgrid.init(min, max, uniformgrid_parts);
      //   else
      //     uniformgrid.reset();
      // }
      // {

      //   uniformgrid.input(particles);
      // }
      // {

      //   uniformgrid.get(tree_container);
      // }
    } break;
  }

  // Check for collisions and resolve if needed

  for (s32 j = 0; j < physics_samples; ++j)
  {
      // Update particles positions
      if (multithreaded_particle_update)
      {
        dispatch.parallel_for_each(position, [dt, this](size_t begin, size_t end)
        {
          for (size_t i = begin; i < end; ++i)
          {
            velocity[i].y -= gravity * mass[i];
            update_particles(begin, end, dt);
          }
        });

      }
      else
      {
          for (size_t i = 0; i < particle_count; ++i)
          {
            velocity[i].y -= gravity * mass[i];
          }
          update_particles(0, particle_count, dt);
      }
    update_collisions();
  }
}

// @CPU
void ParticleSystem::add(const glm::vec2 &pos, float radius, const glm::vec4 &color) noexcept
{
  buffered_call([this, pos, radius, color]()
  {
    glm::vec2 vel;
    if (has_random_velocity)
      vel = rand_vec2(-random_velocity_force, random_velocity_force);

    {
      // @Hack
      if constexpr (multithreaded_engine)
        std::unique_lock<std::mutex> lck(particles_mutex);

      id.emplace_back(particle_count);
      position.emplace_back(pos);
      velocity.emplace_back(vel);
      this->radius.emplace_back(radius);
      mass.emplace_back(particle_density * kPI * radius * radius);
      this->color.emplace_back(color);

      ++particle_count;
    }
  });
}

// @Hot
bool ParticleSystem::collision_check(int a, int b) const noexcept
{
  // Local variables
  const float ax = position[a].x;
  const float ay = position[a].y;
  const float bx = position[b].x;
  const float by = position[b].y;
  const float ar = radius[a];
  const float br = radius[b];

  // square collision check
  if (ax - ar < bx + br &&
      ax + ar > bx - br &&
      ay - ar < by + br &&
      ay + ar > by - br)
  {
    const float dx = bx - ax;
    const float dy = by - ay;

    const float sum_radius = ar + br;
    const float sqr_radius = sum_radius * sum_radius;

    const float distance_sqrd = (dx * dx) + (dy * dy);

    // circle collision check
    return distance_sqrd < sqr_radius;
  }
  return false;
}


// Collisions response between two circles with varying radius and mass.
void ParticleSystem::collision_resolve(int a, int b) noexcept
{
  // Local variables
  auto dx = position[b].x - position[a].x;
  auto dy = position[b].y - position[a].y;
  const auto vdx = velocity[b].x - velocity[a].x;
  const auto vdy = velocity[b].y - velocity[a].y;
  const auto a_vel = velocity[a];
  const auto b_vel = velocity[b];
  const auto m1 = mass[a];
  const auto m2 = mass[b];

  // Should the circles intersect. Seperate them. If not the next
  // calculated values will be off.
  separate(a, b);

  // A negative 'd' means the circles velocities are in opposite directions
  const auto d = dx * vdx + dy * vdy;

  // And we don't resolve collisions between circles moving away from eachother
  if (d < 1e-11f)
  {
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

// Separates two intersecting circles.
void ParticleSystem::separate(int a, int b) noexcept
{
  // Local variables
  const glm::vec2 a_pos = position[a];
  const glm::vec2 b_pos = position[b];
  const f32 ar = radius[a];
  const f32 br = radius[b];

  const f32 collision_depth = (ar + br) - glm::distance(b_pos, a_pos);

  if (collision_depth < 1e-11f) return;

  const f32 dx = b_pos.x - a_pos.x;
  const f32 dy = b_pos.y - a_pos.y;

  // contact angle
  const f32 collision_angle = atan2(dy, dx);
  const f32 cos_angle = cos(collision_angle);
  const f32 sin_angle = sin(collision_angle);

  glm::vec2 a_move = { -collision_depth * 0.5f * cos_angle,  -collision_depth * 0.5f * sin_angle};
  glm::vec2 b_move = {  collision_depth * 0.5f * cos_angle,   collision_depth * 0.5f * sin_angle};

  // stores the position offsets
  glm::vec2 a_pos_move{0.0f};
  glm::vec2 b_pos_move{0.0f};

  // Make sure they dont moved beyond the border
  // This will become not needed when borders are
  //  segments instead of hardcoded.
  if (a_pos.x + a_move.x >= 0.0f + ar &&
      a_pos.x + a_move.x <= framebuffer_width - ar)
    a_pos_move.x += a_move.x;
  if (a_pos.y + a_move.y >= 0.0f + ar &&
      a_pos.y + a_move.y <= framebuffer_height - ar)
    a_pos_move.y += a_move.y;
  if (b_pos.x + b_move.x >= 0.0f + br &&
      b_pos.x + b_move.x <= framebuffer_width - br)
    b_pos_move.x += b_move.x;
  if (b_pos.y + b_move.y >= 0.0f + br &&
      b_pos.y + b_move.y <= framebuffer_height - br)
    b_pos_move.y += b_move.y;

  // Update positions
  position[a] += a_pos_move;
  position[b] += b_pos_move;
}

void ParticleSystem::gravitational_force(int a, int b) noexcept
{
  const f32 x1 = position[a].x;
  const f32 y1 = position[a].y;
  const f32 x2 = position[b].x;
  const f32 y2 = position[b].y;
  const f32 m1 = mass[a];
  const f32 m2 = mass[b];

  const f32 dx = x2 - x1;
  const f32 dy = y2 - y1;
  const f32 d = sqrt(dx * dx + dy * dy);

  const f32 angle = atan2(dy, dx);
  const f64 G = kGravitationalConstant;
  const f32 F = G * m1 * m2 / d * d;

  velocity[a].x += F * cos(angle);
  velocity[a].y += F * sin(angle);
}

void ParticleSystem::apply_n_body() noexcept {
  for (size_t i = 0; i < particle_count; ++i) {
    for (size_t j = 0; j < particle_count; ++j) {
      gravitational_force(i, j);
    }
  }
}

// (N-1)*N/2
void ParticleSystem::collision_logNxN(size_t total, size_t begin, size_t end) noexcept {
  for (size_t i = begin; i < end; ++i) {
    for (size_t j = 1 + i; j < total; ++j) {
      if (collision_check(i, j)) {
        collision_resolve(i, j);
      }
    }
  }
}

void ParticleSystem::collision_quadtree(const vector<vector<s32>> &tree_container, size_t begin, size_t end) noexcept
{
  for (size_t k = begin; k < end; ++k) {
    for (size_t i = 0; i < tree_container[k].size(); ++i) {
      for (size_t j = i + 1; j < tree_container[k].size(); ++j) {
        if (collision_check(tree_container[k][i], tree_container[k][j])){
          collision_resolve(tree_container[k][i], tree_container[k][j]);
        }
      }
    }
  }
}


vector<s32> ParticleSystem::get_neighbours(const Particle& p) const noexcept
{
  vector<vector<s32>> nodes;

  switch (tree_type)
  {
    case TreeType::Quadtree: {quadtree.get_neighbours(nodes, p.pos, p.radius);} break;
    case TreeType::UniformGrid: {uniformgrid.get_neighbours(nodes, p);} break;
    case TreeType::None: { /* Do Nothing */ } break;
  }

  vector<s32> ids;
  for (const auto& node: nodes) {
    ids.reserve(node.size());
    for (const auto& id: node) {
      ids.emplace_back(id);
    }
  }

  return ids;
}


// Returns a vector of ids of particles colliding with the input rectangle.
vector<s32> get_particles_in_rect_basic(const vector<Particle> &particles,
                                        const vec2 &min,
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
vector<s32> get_particles_in_rect(const vector<Particle> &particles,
                                  const vec2 &min, const vec2 &max) noexcept {

  // @Performance: Check for available tree structure used and use that instead.
  vector<s32> ids;
  // Go through all the particles..
  for (const auto &particle : particles) {
    const auto o = particle.pos;
    const auto r = particle.radius;

    // If the particle is inside the rectangle, add it to the output vector.
    if (o.x - r < max.x && o.x + r > min.x && o.y - r < max.y &&
        o.y + r > min.y) {
      ids.emplace_back(particle.id);
    }
  }

  return ids;
}

// Returns a vector of ids of particles colliding with the input rectangle.
vector<s32> ParticleSystem::get_particles_in_circle(const Particle &p) noexcept {

  vector<s32> ids;

 //  // Using a tree
 //  if (tree_type != TreeType::None && circle_collision)
 //  {
 //    vector<s32> potential_collisions;
 //    potential_collisions = get_neighbours(p);
	// if (!potential_collisions.empty())
 //    for (const auto &i : potential_collisions) {
 //      if (collision_check(particles[i], p)) {
 //        ids.emplace_back(i);
 //      }
 //    }
 //  } else {
 //    // Brute-force
 //    for (const auto &i : particles) {
 //      if (collision_check(i, p)) {
 //        ids.emplace_back(i.id);
 //      }
 //    }
 //  }

  return ids;
}

void ParticleSystem::opencl_init() noexcept {
  // Read in the kernel source
  read_file("../Resources/Kernels/particle_collision.cl", &kernel_source);
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

  // Print info
  char device_name[64], driver_version[64], device_version[64];
  u32 val, work_item_dim;
  u64 global_mem_size;
  size_t max_work_group_size, work_item_sizes[3];
  err = clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(char) * 64,
                        &device_name, NULL);
  err = clGetDeviceInfo(device_id, CL_DRIVER_VERSION, sizeof(char) * 64,
                        &driver_version, NULL);
  err = clGetDeviceInfo(device_id, CL_DEVICE_VERSION, sizeof(char) * 64,
                        &device_version, NULL);
  err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint),
                        &val, NULL);
  err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE,
                        sizeof(size_t), &max_work_group_size, NULL);
  err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES,
                        sizeof(size_t) * 3, &work_item_sizes, NULL);
  err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
                        sizeof(cl_uint), &work_item_dim, NULL);
  err = clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong),
                        &global_mem_size, NULL);
  console->info(FRED("OpenCL") " Device name: {}", device_name);
  console->info(FRED("OpenCL") " Device Compute Units: {}", val);
  const auto mem_in_gb = (static_cast<f64>(global_mem_size) / 1073741824.0);
  console->info(FRED("OpenCL") " Device memory: {}GB", mem_in_gb);
  console->info(FRED("OpenCL") " Device supported version: {}", device_version);
  console->info(FRED("OpenCL") " Driver version: {}", driver_version);
  console->info(FRED("OpenCL") " Max workgroup size: {}", max_work_group_size);
  console->info(FRED("OpenCL") " Max workitem sizes: ({},{},{})",
                work_item_sizes[0], work_item_sizes[1], work_item_sizes[2]);
  console->info(FRED("OpenCL") " Max Work item dim: {}", work_item_dim);
}

void ParticleSystem::opencl_naive() noexcept {
  // Create the input and output arrays in device memory
  // for our calculation
  //
  // cl_mem input =
  //     clCreateBuffer(context, CL_MEM_READ_ONLY,
  //                    sizeof(Particle) * (particle_count), NULL, NULL);
  // cl_mem output =
  //     clCreateBuffer(context, CL_MEM_WRITE_ONLY,
  //                    sizeof(Particle) * (particle_count), NULL, NULL);
  // if (!input || !output) {
  //   console->error("Failed to allocate device memory!");
  //   exit(1);
  // }

  // // Write our data set s32o the input array in device
  // // memory
  // err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0,
  //                            sizeof(Particle) * particle_count, &particles[0],
  //                            0, NULL, NULL);

  // if (err != CL_SUCCESS) console->error("Failed to write to source array!");

  // // Set the arguments to our compute kernel
  // err = 0;
  // err = clSetKernelArg(kernel, 0, sizeof(input), &input);
  // err |= clSetKernelArg(kernel, 1, sizeof(output), &output);
  // err |= clSetKernelArg(kernel, 2, sizeof(particle_count), &particle_count);
  // // err |= clSetKernelArg(kernel, 3, sizeof(cl_mem) * local, NULL);
  // if (err != CL_SUCCESS) {
  //   console->error("[line {}] Failed to set kernel arguments! {}", __LINE__,
  //                  err);
  //   exit(1);
  // }

  // // Get the maximum work group size for executing the
  // // kernel on the device
  // err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE,
  //                                sizeof(local), &local, NULL);
  // if (err != CL_SUCCESS) {
  //   console->error("[line {}] Failed to retrieve kernel work group info!{}",
  //                  __LINE__, err);
  //   exit(1);
  // }

  // const auto leftovers = particle_count % local;
  // global_dim = particle_count - leftovers;  // 1D
  // // console->info("OpenCL particle_count: {}", particle_count);
  // // console->info("OpenCL local: {}", local);
  // // console->info("OpenCL global_dim: {}", global_dim);
  // // console->info("OpenCL leftovers run on CPU: {}", leftovers);

  // err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global_dim, &local,
  //                              0, NULL, NULL);
  // // err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global_dim, NULL,
  // // 0, NULL, NULL);
  // if (err) {
  //   console->error("[line {}] Failed to execute kernel! {}", __LINE__, err);
  //   exit(1);
  // }

  // // Wait for the command commands to get serviced before
  // // reading back results
  // err = clFinish(commands);
  // if (err != CL_SUCCESS) {
  //   console->error("[line {}] clFinish! {}", __LINE__, err);
  //   exit(1);
  // }

  // // Read back the results from the device to verify the
  // // output
  // err = clEnqueueReadBuffer(commands, output, CL_TRUE, 0,
  //                           sizeof(Particle) * particle_count, &particles[0], 0,
  //                           NULL, NULL);
  // if (err != CL_SUCCESS) {
  //   console->error("Failed to read output array! {}", err);
  //   exit(1);
  // }

  // if (particle_count != particles.size()) {
  //   console->error("Size of particle_count and particles.size() differ! {}!={}",
  //                  particle_count, particles.size());
  // }

  // // Handle any leftover that werent checked
  // if (leftovers) {
  //   const size_t total = leftovers;
  //   if (use_multithreading) {
  //     // dispatch.parallel_for_each(particles, [this, total](size_t begin, size_t end)
  //     // {
  //     //   collision_logNxN(total, begin, end);
  //     // });
  //   }
  //   else {
  //       collision_logNxN(total, particle_count-total, particle_count);
  //     }
  // }
}

void ParticleSystem::remove_all_with_id(const vector<s32> &ids) noexcept {
  for (const auto id : ids)
  {
    this->id.erase(this->id.begin() + id);
    position.erase(position.begin() + id);
    velocity.erase(velocity.begin() + id);
    radius.erase(radius.begin() + id);
    color.erase(color.begin() + id);
    mass.erase(mass.begin() + id);
  }
  particle_count = position.size();
}

void ParticleSystem::erase_all() noexcept {
  buffered_call([this]()
  {
    if constexpr (multithreaded_engine)
      std::unique_lock<std::mutex> lck(particles_mutex);

    id.clear();
    position.clear();
    velocity.clear();
    radius.clear();
    color.clear();
    mass.clear();

    particle_count = 0;
  });
}

void ParticleSystem::save_state() noexcept
{
  // buffered_call([this]()
  // {
  //     if (particles.empty()) return;


  //     write_data
  //     (
  //       "../bin/data.dat",
  //         particles,
  //         colors,
  //         transforms
  //     );

  //   console->warn("Particle state saved!");
  // });
}

void ParticleSystem::load_state() noexcept
{
  // erase_all();
  // buffered_call([this]()
  // {


  //     read_data(
  //       "../bin/data.dat",
  //         particles,
  //         colors,
  //         transforms);

  //     particle_count = static_cast<u32>(particles.size());

  //     console->warn("Particle state loaded!");
  // });
}

void ParticleSystem::set_particles_color(vector<s32> ids, const vec4& color) noexcept
{
  // buffered_call([this, ids, color]()
  // {
  //   for (auto i: ids)
  //   {
  //     colors[i] = color;
  //   }
  // });
}

static void gravity_well(Particle &a, const vec2 &point) {
  // const f32 x1 = a.pos.x;
  // const f32 y1 = a.pos.y;
  // const f32 x2 = point.x;
  // const f32 y2 = point.y;
  // const f32 m1 = a.mass;
  // const f32 m2 = 1e6f;

  // const f32 dx = x2 - x1;
  // const f32 dy = y2 - y1;
  // const f32 d = sqrt(dx * dx + dy * dy);

  // const f32 angle = atan2(dy, dx);
  // const f64 G = kGravitationalConstant;
  // const f32 F = G * m1 * m2 / d * d;

  // a.acc.x += F * cos(angle);
  // a.acc.y += F * sin(angle);
}

void ParticleSystem::pull_towards_point(const vec2& point) noexcept
{
    buffered_call([this, point]()
    {
      // dispatch.parallel_for_each(particles, [this, point](size_t begin, size_t end)
      // {
      //    for (size_t i = begin; i < end; ++i)
      //     {
      //       gravity_well(particles[i], point);
      //     }
      // });
    });
}
