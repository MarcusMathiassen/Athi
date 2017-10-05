
#include <dispatch/dispatch.h>
#include <cmath>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>

#include <cassert>

#define GLEW_STATIC
#include <GL/glew.h>

#include "athi_camera.h"
#include "athi_circle.h"
#include "athi_quadtree.h"
#include "athi_renderer.h"
#include "athi_settings.h"
#include "athi_spring.h"
#include "athi_voxelgrid.h"

vector<std::function<void()>> circle_update_call_buffer;
std::unique_ptr<Athi_Circle_Manager> athi_circle_manager;

void Athi_Circle::update()
{
  border_collision();

  if (physics_gravity)
    vel.y -= 0.0009f * timestep;

  vel.x += acc.x * timestep;
  vel.y += acc.y * timestep;
  pos.x += vel.x * timestep;
  pos.y += vel.y * timestep;
  acc *= 0;

  transform.pos = glm::vec3(pos.x, pos.y, 0);
}

void Athi_Circle::border_collision()
{
  if (pos.x <= -1.0f + radius && vel.x < 0.0f)
  {
    pos.x = -1.0f + radius;
    vel.x = -vel.x;
  }
  if (pos.x >= 1.0f - radius && vel.x > 0.0f)
  {
    pos.x = 1.0f - radius;
    vel.x = -vel.x;
  }
  if (pos.y <= -1.0f + radius && vel.y < 0.0f)
  {
    pos.y = -1.0f + radius;
    vel.y = -vel.y;
  }
  if (pos.y >= 1.0f - radius && vel.y > 0.0f)
  {
    pos.y = 1.0f - radius;
    vel.y = -vel.y;
  }
}

bool collision_detection(const Athi_Circle &a, const Athi_Circle &b)
{
  const f32 ax = a.pos.x;
  const f32 ay = a.pos.y;
  const f32 bx = b.pos.x;
  const f32 by = b.pos.y;
  const f32 ar = a.radius;
  const f32 br = b.radius;

  // square collision check
  if (ax - ar < bx + br && ax + ar > bx - br && ay - ar < by + br && ay + ar > by - br)
  {
    // circle collision check
    const f32 dx = bx - ax;
    const f32 dy = by - ay;

    const f32 sum_radius = ar + br;
    const f32 sqr_radius = sum_radius * sum_radius;

    const f32 distance_sqr = (dx * dx) + (dy * dy);

    if (distance_sqr <= sqr_radius)
      return true;
  }
  return false;
}

// Collisions response between two circles with varying radius and mass.
void collision_resolve(Athi_Circle &a, Athi_Circle &b)
{
  // Local variables
  const f64 dx = b.pos.x - a.pos.x;
  const f64 dy = b.pos.y - a.pos.y;
  const f64 vdx = b.vel.x - a.vel.x;
  const f64 vdy = b.vel.y - a.vel.y;
  const vec2 a_vel = a.vel;
  const vec2 b_vel = b.vel;
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
  if (d < 0.0)
  {
    const vec2 norm = glm::normalize(vec2(dx, dy));
    const vec2 tang = vec2(norm.y * -1.0, norm.x);
    const f32 scal_norm_1 = glm::dot(norm, a_vel);
    const f32 scal_norm_2 = glm::dot(norm, b_vel);
    const f32 scal_tang_1 = glm::dot(tang, a_vel);
    const f32 scal_tang_2 = glm::dot(tang, b_vel);

    const f32 scal_norm_1_after = (scal_norm_1 * (m1 - m2) + 2.0f * m2 * scal_norm_2) / (m1 + m2);
    const f32 scal_norm_2_after = (scal_norm_2 * (m2 - m1) + 2.0f * m1 * scal_norm_1) / (m1 + m2);
    const vec2 scal_norm_1_after_vec = norm * scal_norm_1_after;
    const vec2 scal_norm_2_after_vec = norm * scal_norm_2_after;
    const vec2 scal_norm_1_vec = tang * scal_tang_1;
    const vec2 scal_norm_2_vec = tang * scal_tang_2;

    // Update velocities
    a.vel = (scal_norm_1_vec + scal_norm_1_after_vec) * 0.99f;
    b.vel = (scal_norm_2_vec + scal_norm_2_after_vec) * 0.99f;
  }
}

// Separates two intersecting circles.
void separate(Athi_Circle &a, Athi_Circle &b)
{
  // Local variables
  const vec2 a_pos = a.pos;
  const vec2 b_pos = b.pos;
  const f32 ar = a.radius;
  const f32 br = b.radius;

  const f32 collision_depth = (ar + br) - glm::distance(b_pos, a_pos);

  const f32 dx = b_pos.x - a_pos.x;
  const f32 dy = b_pos.y - a_pos.y;

  // contact angle
  const f32 collision_angle = atan2(dy, dx);
  const f32 cos_angle = cos(collision_angle);
  const f32 sin_angle = sin(collision_angle);

  // @Note: could this be done using a normal vector and just inverting
  // it? move the balls away from eachother so they dont overlap
  const f32 a_move_x = -collision_depth * 0.5f * cos_angle;
  const f32 a_move_y = -collision_depth * 0.5f * sin_angle;
  const f32 b_move_x = collision_depth * 0.5f * cos_angle;
  const f32 b_move_y = collision_depth * 0.5f * sin_angle;

  // store the new move values
  vec2 a_pos_move;
  vec2 b_pos_move;

  // Make sure they dont moved beyond the border
  if (a_pos.x + a_move_x >= -1.0f + ar && a_pos.x + a_move_x <= 1.0f - ar)
    a_pos_move.x += a_move_x;
  if (a_pos.y + a_move_y >= -1.0f + ar && a_pos.y + a_move_y <= 1.0f - ar)
    a_pos_move.y += a_move_y;
  if (b_pos.x + b_move_x >= -1.0f + br && b_pos.x + b_move_x <= 1.0f - br)
    b_pos_move.x += b_move_x;
  if (b_pos.y + b_move_y >= -1.0f + br && b_pos.y + b_move_y <= 1.0f - br)
    b_pos_move.y += b_move_y;

  // Update positions
  a.pos += a_pos_move;
  b.pos += b_pos_move;
}

void Athi_Circle_Manager::init()
{
  ///////////////////// OPENCL BEGIN /////////////////////
  ///////////////////// OPENCL BEGIN /////////////////////
  ///////////////////// OPENCL BEGIN /////////////////////

  std::cout << "OpenCL initializing..\n";

  read_file("../Resources/circle_collision_kernel.cl", &kernel_source);
  if (kernel_source == nullptr)
    std::cout << "Error: OpenCL missing kernel source. Load it "
                 "before calling "
                 "init().\n";

  // Connect to a compute device
  err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
  if (err != CL_SUCCESS)
    std::cout << "Error: Failed to create a device group!\n";

  // Create a compute context
  context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
  if (!context)
    std::cout << "Error: Failed to create a compute context!\n";

  // Create a command commands
  commands = clCreateCommandQueue(context, device_id, 0, &err);
  if (!commands)
    std::cout << "Error: Failed to create a command commands!\n";

  // Create the compute program from the source buffer
  program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, NULL, &err);
  if (!program)
    std::cout << "Error: Failed to create compute program!\n";

  // Build the program executable
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS)
  {
    size_t len;
    char buffer[2048];

    std::cout << "Error: Failed to build program executable!\n";
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
    std::cout << buffer << '\n';
  }

  // Create the compute kernel in the program we wish to run
  kernel = clCreateKernel(program, "hello", &err);
  if (!kernel || err != CL_SUCCESS)
    std::cout << "Error: Failed to create compute kernel!\n";

  ///////////////////// SHADER BEGIN  /////////////////////
  ///////////////////// SHADER BEGIN  /////////////////////
  ///////////////////// SHADER BEGIN  /////////////////////

  shader_program = glCreateProgram();
  const u32 vs = createShader("../Resources/athi_circle_shader.vs", GL_VERTEX_SHADER);
  const u32 fs = createShader("../Resources/athi_circle_shader.fs", GL_FRAGMENT_SHADER);

  glAttachShader(shader_program, vs);
  glAttachShader(shader_program, fs);

  glBindAttribLocation(shader_program, 0, "position");
  glBindAttribLocation(shader_program, 1, "color");
  glBindAttribLocation(shader_program, 2, "transform");

  glLinkProgram(shader_program);
  glValidateProgram(shader_program);
  validateShaderProgram("circle_manager", shader_program);

  glDetachShader(shader_program, vs);
  glDetachShader(shader_program, fs);
  glDeleteShader(vs);
  glDeleteShader(fs);

  ///////////////////// MESH BEGIN  /////////////////////
  ///////////////////// MESH BEGIN  /////////////////////
  ///////////////////// MESH BEGIN  /////////////////////

  std::vector<vec2> positions;
  positions.reserve(CIRCLE_NUM_VERTICES);
  for (u32 i = 0; i < CIRCLE_NUM_VERTICES; ++i)
  {
    positions.emplace_back(std::cos(i * M_PI * 2.0f / CIRCLE_NUM_VERTICES), std::sin(i * M_PI * 2.0f / CIRCLE_NUM_VERTICES));
  }

  ////////////////////// VAO/VBO BEGIN  /////////////////////
  ////////////////////// VAO/VBO BEGIN  /////////////////////
  ////////////////////// VAO/VBO BEGIN  /////////////////////

  // VAO
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // VBO
  glGenBuffers(NUM_BUFFERS, VBO);

  // POSITION
  glBindBuffer(GL_ARRAY_BUFFER, VBO[POSITION]);
  glBufferData(GL_ARRAY_BUFFER, CIRCLE_NUM_VERTICES * sizeof(positions[0]), &positions[0], GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  // COLOR
  glBindBuffer(GL_ARRAY_BUFFER, VBO[COLOR]);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glVertexAttribDivisor(1, 1);

  // TRANSFORM
  glBindBuffer(GL_ARRAY_BUFFER, VBO[TRANSFORM]);
  for (u32 i = 0; i < 4; ++i)
  {
    glEnableVertexAttribArray(2 + i);
    glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (GLvoid *)(i * sizeof(glm::vec4)));
    glVertexAttribDivisor(2 + i, 1);
  }
}

Athi_Circle_Manager::~Athi_Circle_Manager()
{
  glDeleteBuffers(NUM_BUFFERS, VBO);
  glDeleteVertexArrays(1, &VAO);

  ////////////////////// OPENCL CLEANUP /////////////////////
  ////////////////////// OPENCL CLEANUP /////////////////////
  ////////////////////// OPENCL CLEANUP /////////////////////
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseContext(context);
  clReleaseMemObject(input);
  clReleaseMemObject(output);
  clReleaseCommandQueue(commands);

  std::cout << "OpenCL shutting down..\n";
}

void Athi_Circle_Manager::draw()
{
  if (circle_buffer.empty())
    return;

  glBindBuffer(GL_ARRAY_BUFFER, VBO[TRANSFORM]);
  size_t transform_bytes_needed = sizeof(mat4) * circle_buffer.size();
  if (transform_bytes_needed > transform_bytes_allocated)
  {
    glBufferData(GL_ARRAY_BUFFER, transform_bytes_needed, &transforms[0], GL_STREAM_DRAW);
    transform_bytes_allocated = transform_bytes_needed;
  }
  else
  {
    glBufferSubData(GL_ARRAY_BUFFER, 0, transform_bytes_allocated, &transforms[0]);
  }

  glBindBuffer(GL_ARRAY_BUFFER, VBO[COLOR]);
  size_t color_bytes_needed = sizeof(vec4) * circle_buffer.size();
  if (color_bytes_needed > color_bytes_allocated)
  {
    glBufferData(GL_ARRAY_BUFFER, color_bytes_needed, &colors[0], GL_STREAM_DRAW);
    color_bytes_allocated = color_bytes_needed;
  }
  else
  {
    glBufferSubData(GL_ARRAY_BUFFER, 0, color_bytes_allocated, &colors[0]);
  }

  glBindVertexArray(VAO);
  glUseProgram(shader_program);
  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, CIRCLE_NUM_VERTICES, (s32)circle_buffer.size());
}

void Athi_Circle_Manager::update()
{
  if (circle_buffer.empty())
    return;
  if (circle_collision)
  {
    std::vector<std::vector<size_t>> cont;

    if (quadtree_active && openCL_active == false)
    {
      quadtree = Quadtree<Athi_Circle>(quadtree_depth, quadtree_capacity, vec2(-1, -1), vec2(1, 1));
      quadtree.input(circle_buffer);
      quadtree.get(cont);
    }
    else if (voxelgrid_active && openCL_active == false)
    {
      update_voxelgrid();
      get_nodes_voxelgrid(cont);
    }
    // Quadtree or Voxelgrid
    if ((quadtree_active || voxelgrid_active) && openCL_active == false)
    {
      if (use_multithreading && variable_thread_count != 0)
      {
        const u32 thread_count = variable_thread_count;
        const size_t total = cont.size();
        const size_t parts = total / thread_count;

        collision_quadtree(cont, parts * thread_count, total);

        dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

        dispatch_apply(thread_count, queue, ^(size_t i) {
          const size_t begin = parts * i;
          const size_t end = parts * (i + 1);
          collision_quadtree(cont, begin, end);
          ++i;
        });
      }
      else
        collision_quadtree(cont, 0, cont.size());
    }
    else if (use_multithreading && variable_thread_count != 0 && openCL_active == false)
    {
      const u32 thread_count = variable_thread_count;
      const size_t total = circle_buffer.size();
      const size_t parts = total / thread_count;

      collision_logNxN(total, parts * thread_count, total);

      dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

      dispatch_apply(thread_count, queue, ^(size_t i) {
        const size_t begin = parts * i;
        const size_t end = parts * (i + 1);
        collision_logNxN(total, begin, end);
        ++i;
      });
    }

    ////////////////////// OPENCL UPDATE BEGIN /////////////////////
    ////////////////////// OPENCL UPDATE BEGIN /////////////////////
    ////////////////////// OPENCL UPDATE BEGIN /////////////////////
    else if (openCL_active)
    {
      const auto count = circle_buffer.size();

      data.clear();
      results.clear();
      data.resize(count);
      results.resize(count);

      // Copy over the results
      for (size_t i = 0; i < count; ++i)
      {
        data[i] = circle_buffer[i];
      }

      // Create the input and output arrays in device memory
      // for our calculation
      //
      input = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(data), NULL, NULL);
      output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(results), NULL, NULL);
      if (!input || !output)
      {
        std::cout << "Error: Failed to allocate device "
                     "memory!\n";
        exit(1);
      }

      // Write our data set into the input array in device
      // memory
      err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(data), &data[0], 0, NULL, NULL);
      if (err != CL_SUCCESS)
        printf(
            "Error: Failed to write to source "
            "array!\n");

      // Set the arguments to our compute kernel
      err = 0;
      err = clSetKernelArg(kernel, 0, sizeof(input), &input);
      err |= clSetKernelArg(kernel, 1, sizeof(output), &output);
      err |= clSetKernelArg(kernel, 2, sizeof(count), &count);
      if (err != CL_SUCCESS)
      {
        std::cout << "Error: Failed to set kernel arguments! " << err << '\n';
        exit(1);
      }

      // Get the maximum work group size for executing the
      // kernel o dn the device
      //
      err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
      if (err != CL_SUCCESS)
      {
        std::cout << "Error: Failed to retrieve kernel "
                     "work group info! "
                  << err << '\n';
        exit(1);
      }

      global = count;
      // err = clEnqueueNDRangeKernel(commands, kernel, 1,
      // NULL, &global, &local, 0, NULL, NULL);
      err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
      if (err)
      {
        std::cout << "Error: Failed to execute kernel! " << err << '\n';
        exit(1);
      }

      // Wait for the command commands to get serviced before
      // reading back results
      clFinish(commands);

      // Read back the results from the device to verify the
      // output
      err = clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(results), &results[0], 0, NULL, NULL);
      if (err != CL_SUCCESS)
      {
        std::cout << "Error: Failed to read output array! " << err << '\n';
        exit(1);
      }

      clFinish(commands);

      // Copy over the results
      for (size_t i = 0; i < count; ++i)
      {
        circle_buffer[i].pos = results[i].pos;
        circle_buffer[i].vel = results[i].vel;
        circle_buffer[i].color = results[i].color;
      }
    }
    ////////////////////// OPENCL UPDATE END  /////////////////////
    ////////////////////// OPENCL UPDATE END  /////////////////////
    ////////////////////// OPENCL UPDATE END  /////////////////////

    // CPU Singlethreaded
    else
    {
      collision_logNxN(circle_buffer.size(), 0, circle_buffer.size());
    }
  }

  // Update GPU BUFFERS
  if (circle_buffer.size() > transforms.size())
  {
    transforms.resize(circle_buffer.size());
    colors.resize(circle_buffer.size());
  }

  size_t i = 0;
  for (const auto &circle : circle_buffer)
  {
    transforms[i] = circle.transform.get_model() * camera.get_view_projection();
    colors[i++] = circle.color;
  }
}

void Athi_Circle_Manager::collision_logNxN(size_t total, size_t begin, size_t end)
{
  for (size_t i = begin; i < end; ++i)
  {
    for (size_t j = 1 + i; j < total; ++j)
    {
      if (collision_detection(circle_buffer[i], circle_buffer[j]))
      {
        collision_resolve(circle_buffer[i], circle_buffer[j]);
      }
    }
  }
}

void Athi_Circle_Manager::collision_quadtree(const std::vector<std::vector<size_t>> &cont, size_t begin, size_t end)
{
  for (size_t k = begin; k < end; ++k)
  {
    for (size_t i = 0; i < cont[k].size(); ++i)
    {
      for (size_t j = i + 1; j < cont[k].size(); ++j)
      {
        if (collision_detection(circle_buffer[cont[k][i]], circle_buffer[cont[k][j]]))
        {
          collision_resolve(circle_buffer[cont[k][i]], circle_buffer[cont[k][j]]);
        }
      }
    }
  }
}

void Athi_Circle_Manager::add_circle_multiple(Athi_Circle &circle, int num)
{
  std::lock_guard<std::mutex> lock(circle_buffer_function_mutex);
  for (int i = 0; i < num; ++i)
  {
    circle.id = (u32)circle_buffer.size();
    circle_buffer.emplace_back(circle);
  }
}

void Athi_Circle_Manager::add_circle(Athi_Circle &circle)
{
  std::lock_guard<std::mutex> lock(circle_buffer_function_mutex);
  circle.id = (u32)circle_buffer.size();
  circle_buffer.emplace_back(circle);
}

Athi_Circle Athi_Circle_Manager::get_circle(u32 id)
{
  std::lock_guard<std::mutex> lock(circle_buffer_function_mutex);
  return circle_buffer[id];
}

void Athi_Circle_Manager::update_circles()
{
  std::lock_guard<std::mutex> lock(circle_buffer_function_mutex);
  update();
  for (auto &circle : circle_buffer)
    circle.update();
  update_springs();

  for (auto &c : circle_update_call_buffer)
    c();
  circle_update_call_buffer.clear();
}

void Athi_Circle_Manager::draw_circles()
{
  std::lock_guard<std::mutex> lock(circle_buffer_function_mutex);

  if (voxelgrid_active && draw_debug)
    draw_voxelgrid();
  if (quadtree_active && draw_debug) 
    quadtree.color_objects(circle_buffer);
  draw();

  if (clear_circles)
  {
    circle_buffer.clear();
    circle_buffer.shrink_to_fit();
    clear_circles = false;
    spring_buffer.clear();
  }
}

void Athi_Circle_Manager::set_color_circle_id(u32 id, const vec4 &color) { circle_buffer[id].color = color; }

void init_circle_manager()
{
  athi_circle_manager = std::make_unique<Athi_Circle_Manager>();
  athi_circle_manager->init();
}

void update_circles() { athi_circle_manager->update_circles(); }
void draw_circles() { athi_circle_manager->draw_circles(); }
void delete_circles() { athi_circle_manager->clear_circles = true; }

void update_circle_call(const std::function<void()> &f)
{
  std::lock_guard<std::mutex> lock(athi_circle_manager->circle_buffer_function_mutex);
  circle_update_call_buffer.emplace_back(std::move(f));
}

void add_circle_multiple(Athi_Circle &circle, int num)
{
  const f32 radi = circle.radius;
  circle.mass = (1.33333f * M_PI * radi * radi * radi);
  circle.transform.scale = glm::vec3(radi, radi, 0);
  athi_circle_manager->add_circle_multiple(circle, num);
}

void add_circle(Athi_Circle &circle)
{
  const f32 radi = circle.radius;
  circle.mass = (1.33333f * M_PI * radi * radi * radi);
  circle.transform.scale = glm::vec3(radi, radi, 0);
  athi_circle_manager->add_circle(circle);
}

u32 get_num_circles() { return (u32)athi_circle_manager->circle_buffer.size(); }
