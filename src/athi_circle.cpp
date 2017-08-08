#include "athi_circle.h"
#include "athi_settings.h"
#include "athi_quadtree.h"
#include "athi_voxelgrid.h"
#include "athi_camera.h"

#include <iostream>
#include <cmath>
#include <thread>
#include <glm/gtx/vector_angle.hpp>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

std::vector<u32> leftover_circles;
std::vector<std::unique_ptr<Athi_Circle> > circle_buffer;
std::unique_ptr<Athi_Circle_Manager> athi_circle_manager;

void Athi_Circle::update()
{
  border_collision();

  if (physics_gravity) vel.y -= (9.81f) * 0.0001f * timestep;

  pos.x += vel.x * timestep;
  pos.y += vel.y * timestep;

  transform.pos   = glm::vec3(pos.x, pos.y, 0);
  transform.scale = glm::vec3(radius, radius, 0);
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
    pos.x = (1.0f - radius);
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


bool collisionDetection(u32 a_id, u32 b_id)
{
  auto a = *athi_circle_manager->circle_buffer[a_id];
  auto b = *athi_circle_manager->circle_buffer[b_id];

  const f32 ax = a.pos.x;
  const f32 ay = a.pos.y;
  const f32 bx = b.pos.x;
  const f32 by = b.pos.y;
  const f32 ar = a.radius;
  const f32 br = b.radius;

  // square collision check
  if (ax - ar < bx + br &&
      ax + ar > bx - br &&
      ay - ar < by + br &&
      ay + ar > by - br)
  {
    // circle collision check
    const f32 dx = bx - ax;
    const f32 dy = by - ay;

    const f32 sum_radius = ar + br;
    const f32 sqr_radius = sum_radius * sum_radius;

    const f32 distance_sqr = (dx * dx) + (dy * dy);

    if (distance_sqr <= sqr_radius) return true;
  }

  return false;
}

void collisionResolve(u32 a_id, u32 b_id)
{
  auto a = *athi_circle_manager->circle_buffer[a_id];
  auto b = *athi_circle_manager->circle_buffer[b_id];

  separate(a_id, b_id);

  const f64 dx      = b.pos.x - a.pos.x;
  const f64 dy      = b.pos.y - a.pos.y;
  const f64 vdx     = b.vel.x - a.vel.x;
  const f64 vdy     = b.vel.y - a.vel.y;
  const vec2 a_vel  = a.vel;
  const vec2 b_vel  = b.vel;
  const f32 m1      = a.mass;
  const f32 m2      = b.mass;

  const f64 d = dx * vdx + dy * vdy;
 
  // skip if they're moving away from eachother
  if (d < 0.0)
  {
    const vec2 norm       = glm::normalize(vec2(dx, dy));
    const vec2 tang       = vec2(norm.y * -1.0, norm.x);
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

    athi_circle_manager->circle_buffer[a_id]->vel = (scal_norm_1_vec + scal_norm_1_after_vec) * 0.95f;
    athi_circle_manager->circle_buffer[b_id]->vel = (scal_norm_2_vec + scal_norm_2_after_vec) * 0.95f;
  }
}

// Separates two intersecting circles.
void separate(u32 a_id, u32 b_id)
{
  auto a = *athi_circle_manager->circle_buffer[a_id];
  auto b = *athi_circle_manager->circle_buffer[b_id];

  const vec2 a_pos = a.pos;
  const vec2 b_pos = b.pos;
  const f32  ar    = a.radius;
  const f32  br    = b.radius;

  const f32 collision_depth = (ar + br) - glm::distance(b_pos, a_pos);

  const f32 dx = b_pos.x - a_pos.x;
  const f32 dy = b_pos.y - a_pos.y;

  // contact angle
  const f32 collision_angle = atan2(dy, dx);
  const f32 cos_angle       = cos(collision_angle);
  const f32 sin_angle       = sin(collision_angle);

  // move the balls away from eachother so they dont overlap
  const f32 a_move_x = -collision_depth * 0.5f * cos_angle;
  const f32 a_move_y = -collision_depth * 0.5f * sin_angle;
  const f32 b_move_x =  collision_depth * 0.5f * cos_angle;
  const f32 b_move_y =  collision_depth * 0.5f * sin_angle;

  // store the new move values
  vec2 a_pos_move;
  vec2 b_pos_move;

  // Make sure they dont moved beyond the border
  if (a_pos.x + a_move_x >= -1.0f + ar && a_pos.x + a_move_x <= 1.0f - ar) a_pos_move.x += a_move_x;
  if (a_pos.y + a_move_y >= -1.0f + ar && a_pos.y + a_move_y <= 1.0f - ar) a_pos_move.y += a_move_y;
  if (b_pos.x + b_move_x >= -1.0f + br && b_pos.x + b_move_x <= 1.0f - br) b_pos_move.x += b_move_x;
  if (b_pos.y + b_move_y >= -1.0f + br && b_pos.y + b_move_y <= 1.0f - br) b_pos_move.y += b_move_y;

  // Update.
  athi_circle_manager->circle_buffer[a_id]->pos += a_pos_move;
  athi_circle_manager->circle_buffer[b_id]->pos += b_pos_move;
}


void Athi_Circle_Manager::init()
{
  shader_program  = glCreateProgram();
  const u32 vs   = createShader("../Resources/athi_circle_shader.vs", GL_VERTEX_SHADER);
  const u32 fs   = createShader("../Resources/athi_circle_shader.fs", GL_FRAGMENT_SHADER);

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

  // MESH SETUP
  std::vector<vec2> positions;
  positions.reserve(CIRCLE_NUM_VERTICES);
  for (u32 i = 0; i < CIRCLE_NUM_VERTICES; ++i)
  {
    positions.emplace_back(cos(i * M_PI * 2.0f / CIRCLE_NUM_VERTICES),
                           sin(i * M_PI * 2.0f / CIRCLE_NUM_VERTICES));
  }

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
  for (u32 i = 0; i < 4; ++i) {
    glEnableVertexAttribArray(2 + i);
    glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                          (GLvoid *)(i * sizeof(glm::vec4)));
    glVertexAttribDivisor(2 + i, 1);
  }
}

Athi_Circle_Manager::~Athi_Circle_Manager()
{
  glDeleteBuffers(NUM_BUFFERS, VBO);
  glDeleteVertexArrays(1, &VAO);
}

void Athi_Circle_Manager::draw()
{
  if (circle_buffer.empty()) return;

  if (circle_buffer.size() > transforms.size())
  {
    transforms.resize(circle_buffer.size());
    colors.resize(circle_buffer.size());
  }

  u32 i = 0;
  for (const auto &circle : circle_buffer)
  {
    transforms[i] = circle->transform.get_model() * camera.get_view_projection();
    colors[i++] = circle->color;
  }

  glBindBuffer(GL_ARRAY_BUFFER, VBO[TRANSFORM]);
  size_t transform_bytes_needed = sizeof(mat4) * circle_buffer.size();
  if (transform_bytes_needed > transform_bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, transform_bytes_needed, &transforms[0], GL_STREAM_DRAW);
    transform_bytes_allocated = transform_bytes_needed;
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, transform_bytes_allocated, &transforms[0]);
  }

  glBindBuffer(GL_ARRAY_BUFFER, VBO[COLOR]);
  size_t color_bytes_needed = sizeof(vec4) * circle_buffer.size();
  if (color_bytes_needed > color_bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, color_bytes_needed, &colors[0], GL_STREAM_DRAW);
    color_bytes_allocated = color_bytes_needed;
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, color_bytes_allocated, &colors[0]);
  }

  glBindVertexArray(VAO);
  glUseProgram(shader_program);
  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, CIRCLE_NUM_VERTICES, (s32)circle_buffer.size());
}

void Athi_Circle_Manager::update()
{
  if (circle_buffer.size() == 0) return;
  if (circle_collision)
  {
    std::vector<std::vector<u32> > cont;

    if (quadtree_active)
    {
      update_quadtree();
      get_nodes_quadtree(cont);
    }
    else if (voxelgrid_active)
    {
      update_voxelgrid();
      get_nodes_voxelgrid(cont);
    }
    if (quadtree_active || voxelgrid_active)
    {
      // multithreaded

      // single threaded
      collision_quadtree(cont, 0, cont.size());

      // Take care of the leftovers
       if (use_multithreading && variable_thread_count != 0)
       {
          // const u32 thread_count = variable_thread_count;
          // const size_t total = leftover_circles.size();
          // const u32 parts = total / thread_count;

          // threads.resize(thread_count);

          // collision_logNxN(total, parts * thread_count, total);

          // u32 i = 0;
          // for (auto &thread: threads)
          // {
          //   thread = std::thread(&Athi_Circle_Manager::collision_logNxN, this, total, parts * i, parts * (1 + i));
          //   ++i;
          // }
          // for (auto &thread : threads) thread.join();
       } else collision_logNxN_leftover(circle_buffer.size(), 0, leftover_circles.size());
    }

    if (use_multithreading && variable_thread_count != 0)
    {
      const u32 thread_count = variable_thread_count;
      const size_t total = circle_buffer.size();
      const u32 parts = total / thread_count;

      threads.resize(thread_count);

      collision_logNxN(total, parts * thread_count, total);
      //collision_quadtree(cont, cont.size()-1, cont.size());

      u32 i = 0;
      for (auto &thread: threads)
      {
        thread = std::thread(&Athi_Circle_Manager::collision_logNxN, this, total, parts * i, parts * (1 + i));
        //thread = std::thread(&Athi_Circle_Manager::collision_quadtree, this, cont, parts * i, parts * (1 + i));
        ++i;
      }
      for (auto &thread : threads) thread.join();
    }
    else
    {
      // total, start, end
      collision_logNxN(circle_buffer.size(), 0, circle_buffer.size());
    }
  }

  for (auto &circle : circle_buffer)
  {
    circle->update();
  }
}

void Athi_Circle_Manager::collision_logNxN_leftover(size_t total, size_t begin, size_t end)
{
  for (size_t i = begin; i < end; ++i) {
    for (size_t j = 0; j < total; ++j) {
      if (collisionDetection(leftover_circles[i], j))
      {
          collisionResolve(leftover_circles[i], j);
      }
      ++comparisons;
    }
  }
}

void Athi_Circle_Manager::collision_logNxN(size_t total, size_t begin, size_t end)
{
  for (size_t i = begin; i < end; ++i) {
    for (size_t j = 1 + i; j < total; ++j) {
      if (collisionDetection(i, j)) {
          collisionResolve(i, j);
      }
      ++comparisons;
    }
  }
}

void Athi_Circle_Manager::collision_quadtree(const std::vector<std::vector<u32> > &cont, size_t begin, size_t end)
{
  for (size_t k = begin; k < end; ++k) {
    for (size_t i = 0; i < cont[k].size(); ++i) {
      for (size_t j = i + 1; j < cont[k].size(); ++j) {
        if (collisionDetection(cont[k][i], cont[k][j])) {
            collisionResolve(cont[k][i], cont[k][j]);
        }
        ++comparisons;
      }
    }
  }
}

void Athi_Circle_Manager::add_circle(Athi_Circle& circle)
{
  std::lock_guard<std::mutex> lock(circle_buffer_function_mutex);
  circle.id = circle_buffer.size();
  circle_buffer.emplace_back(std::make_unique<Athi_Circle>(circle));
}

Athi_Circle Athi_Circle_Manager::get_circle(u32 id)
{
  std::lock_guard<std::mutex> lock(circle_buffer_function_mutex);
  return *circle_buffer[id];
}

void Athi_Circle_Manager::update_circles()
{
  std::lock_guard<std::mutex> lock(circle_buffer_function_mutex);
  update();
  std::cout << "leftover circles: " << leftover_circles.size() << std::endl;
  leftover_circles.clear();
}

void Athi_Circle_Manager::draw_circles()
{
  std::lock_guard<std::mutex> lock(circle_buffer_function_mutex);

  draw();
  if (voxelgrid_active && draw_debug) draw_voxelgrid();
  if (quadtree_active && draw_debug)  draw_quadtree();

  if (clear_circles)
  {
    circle_buffer.clear();
    clear_circles = false;
    reset_quadtree();
  }
  comparisons = 0;
}

void Athi_Circle_Manager::set_color_circle_id(u32 id, const vec4& color)
{
  circle_buffer[id]->color = color;
}

void init_circle_manager()
{
  athi_circle_manager = std::make_unique<Athi_Circle_Manager>();
  athi_circle_manager->init();
}

void update_circles()
{
  athi_circle_manager->update_circles();
}
void draw_circles()
{
  athi_circle_manager->draw_circles();
}

void delete_circles()
{
  athi_circle_manager->clear_circles = true;
}

void add_circle(Athi_Circle &circle)
{
  circle.mass = 1.33333f * M_PI * circle.radius * circle.radius * circle.radius;
  athi_circle_manager->add_circle(circle);
}

u32 get_num_circles()
{
  return (u32)athi_circle_manager->circle_buffer.size();
}
