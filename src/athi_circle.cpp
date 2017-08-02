#include "athi_circle.h"
#include "athi_settings.h"
#include "athi_quadtree.h"
#include "athi_voxelgrid.h"

#include <cmath>
#include <thread>
#include <glm/gtx/vector_angle.hpp>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

std::vector<Athi_Circle> circle_buffer;

void Athi_Circle::update()
{
  borderCollision();

  if (physics_gravity) vel.y -= (9.81f * mass) * timestep;

  pos.x += vel.x * timestep;
  pos.y += vel.y * timestep;

  transform.pos   = glm::vec3(pos.x, pos.y, 0);
  transform.scale = glm::vec3(radius, radius, 0);
}

void Athi_Circle::borderCollision() {
  if (pos.x <= -1.0f + radius && vel.x < 0.0f) {  // LEFT
    pos.x = -1.0f + radius;
    vel.x = -vel.x;
  }
  if (pos.x >= 1.0f - radius && vel.x > 0.0f) {  // RIGHT
    pos.x = (1.0f - radius);
    vel.x = -vel.x;
  }
  if (pos.y <= -1.0f + radius && vel.y < 0.0f) {  // BOTTOM
    pos.y = -1.0f + radius;
    vel.y = -vel.y;
  }
  if (pos.y >= 1.0f - radius && vel.y > 0.0f) {  // TOP
    pos.y = 1.0f - radius;
    vel.y = -vel.y;
  }
}


static bool collisionDetection(const Athi_Circle &a, const Athi_Circle &b)
{
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
      ay + ar > by - br) {

    // circle collision check
    const f32 dx = bx - ax;
    const f32 dy = by - ay;

    const f32 sumRadius = ar + br;
    const f32 sqrRadius = sumRadius * sumRadius;

    const f32 distSqr = (dx * dx) + (dy * dy);

    if (distSqr <= sqrRadius) return true;
  }
  return false;
}

static void collisionResolve(Athi_Circle &a, Athi_Circle &b)
{
  separate(a, b);

  const f64 dx = b.pos.x - a.pos.x;
  const f64 dy = b.pos.y - a.pos.y;
  const f64 vdx = b.vel.x - a.vel.x;
  const f64 vdy = b.vel.y - a.vel.y;
  const f64 d = dx * vdx + dy * vdy;

  // if they're not moving away from eachother
  if (d < 0.0) {
    const vec2 norm = glm::normalize(vec2(dx,dy));
    const vec2 tang{norm.y * -1.0, norm.x};
    const f32 scal_norm_1 = glm::dot(norm, a.vel);
    const f32 scal_norm_2 = glm::dot(norm, b.vel);
    const f32 scal_tang_1 = glm::dot(tang, a.vel);
    const f32 scal_tang_2 = glm::dot(tang, b.vel);

    const f32 m1 = a.mass;
    const f32 m2 = b.mass;

    const f32 scal_norm_1_after = (scal_norm_1 * (m1 - m2) + 2.0f * m2 * scal_norm_2) / (m1 + m2);
    const f32 scal_norm_2_after = (scal_norm_2 * (m2 - m1) + 2.0f * m1 * scal_norm_1) / (m1 + m2);
    const vec2 scal_norm_1_after_vec{norm * scal_norm_1_after};
    const vec2 scal_norm_2_after_vec{norm * scal_norm_2_after};
    const vec2 scal_norm_1_vec{tang * scal_tang_1};
    const vec2 scal_norm_2_vec{tang * scal_tang_2};

    a.vel = (scal_norm_1_vec + scal_norm_1_after_vec)*0.99f;
    b.vel = (scal_norm_2_vec + scal_norm_2_after_vec)*0.99f;
  }
}

// Separates two intersecting circles.
static void separate(Athi_Circle &a, Athi_Circle &b)
{
  const vec2 apos{a.pos};
  const vec2 bpos{b.pos};
  const f32 ar{a.radius};
  const f32 br{b.radius};

  const f32 colDepth = (ar + br) - glm::distance(b.pos, a.pos);

  const f32 dx = bpos.x - apos.x;
  const f32 dy = bpos.y - apos.y;

  // contact angle
  const f32 colAngle = atan2(dy, dx);
  const f32 cos_angle = cos(colAngle);
  const f32 sin_angle = sin(colAngle);

  // move the balls away from eachother so they dont overlap
  const f32 a_move_x = -colDepth * 0.5f * cos_angle;
  const f32 a_move_y = -colDepth * 0.5f * sin_angle;
  const f32 b_move_x = colDepth * 0.5f * cos_angle;
  const f32 b_move_y = colDepth * 0.5f * sin_angle;

  // Make sure they dont get moved beyond the border
  if (apos.x + a_move_x >= -1.0f + ar && apos.x + a_move_x <= 1.0f - ar) {
    a.pos.x += a_move_x;
  }
  if (apos.y + a_move_y >= -1.0f + ar && apos.y + a_move_y <= 1.0f - ar) {
    a.pos.y += a_move_y;
  }
  if (bpos.x + b_move_x >= -1.0f + br && bpos.x + b_move_x <= 1.0f - br) {
    b.pos.x += b_move_x;
  }
  if (bpos.y + b_move_y >= -1.0f + br && bpos.y + b_move_y <= 1.0f - br) {
    b.pos.y += b_move_y;
  }
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
  for (int i = 0; i < CIRCLE_NUM_VERTICES; ++i)
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
  glBufferData(GL_ARRAY_BUFFER, CIRCLE_NUM_VERTICES * sizeof(positions[0]), &positions[0], GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

  // COLOR
  glBindBuffer(GL_ARRAY_BUFFER, VBO[COLOR]);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glVertexAttribDivisor(1, 1);

  // TRANSFORM
  glBindBuffer(GL_ARRAY_BUFFER, VBO[TRANSFORM]);
  for (int i = 0; i < 4; ++i) {
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
  if (circle_buffer.size() == 0) return;

  if (circle_buffer.size() > transforms.size())
  {
    transforms.resize(circle_buffer.size());
    colors.resize(circle_buffer.size());
  }

  // Update the color and transform arrays
  u32 i = 0;
  for (const auto &circle : circle_buffer)
  {
    transforms[i] = circle.transform.get_model();
    colors[i++] = circle.color;
  }

  // transforms BUFFER
  glBindBuffer(GL_ARRAY_BUFFER, VBO[TRANSFORM]);
  // Does the buffer need to allocate more space?
  size_t transform_bytes_needed = sizeof(mat4) * circle_buffer.size();
  if (transform_bytes_needed > transform_bytes_allocated) {
    glBufferData(GL_ARRAY_BUFFER, transform_bytes_needed, &transforms[0], GL_STREAM_DRAW);
    transform_bytes_allocated = transform_bytes_needed;
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, transform_bytes_allocated, &transforms[0]);
  }

  // COLOR BUFFER
  glBindBuffer(GL_ARRAY_BUFFER, VBO[COLOR]);
  // Does the buffer need to allocate more space?
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
  for (auto &circle : circle_buffer) circle.update();

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
    if (quadtree_active || voxelgrid_active) collision_quadtree(cont, 0, cont.size());
    else if (use_multithreading && variable_thread_count != 0)
    {
      const u32 total = (u32)circle_buffer.size();
      const u32 parts = total / variable_thread_count;

      u32 const thread_count = variable_thread_count;
      std::thread thread_pool[thread_count];

      collision_logNxN(parts * thread_count, total);
      for (u32 i = 0; i < thread_count; ++i)
      {
        thread_pool[i] = std::thread(&Athi_Circle_Manager::collision_logNxN, this, parts * i, parts * (i + 1));
      }
      for (auto &thread : thread_pool) thread.join();
    }
    else
    {
      collision_logNxN(0, circle_buffer.size());
    }
  }
}

void Athi_Circle_Manager::collision_logNxN(size_t begin, size_t end)
{
  for (size_t i = begin; i < end; ++i)
    for (size_t j = 1 + i; j < circle_buffer.size(); ++j)
      if (collisionDetection( circle_buffer[i],   circle_buffer[j]))
          collisionResolve(   circle_buffer[i],   circle_buffer[j]);
}

void Athi_Circle_Manager::collision_quadtree(const std::vector<std::vector<u32> > &cont, size_t begin, size_t end)
{
  for (size_t k = begin; k < end; ++k)
    for (size_t i = 0; i < cont[k].size(); ++i)
      for (size_t j = i + 1; j < cont[k].size(); ++j)
        if (collisionDetection( circle_buffer[cont[k][i]],  circle_buffer[cont[k][j]]))
            collisionResolve(   circle_buffer[cont[k][i]],  circle_buffer[cont[k][j]]);
}

void init_circle_manager()
{
  athi_circle_manager.init();
}

void update_circles()
{
  athi_circle_manager.update();
}
void draw_circles()
{
  athi_circle_manager.draw();
}

void delete_circles()
{
  circle_buffer.clear();
}

void addCircle(Athi_Circle &circle)
{
  circle.mass = 1.33333f * M_PI * circle.radius * circle.radius * circle.radius;
  circle.id = (u32)circle_buffer.size();
  circle_buffer.emplace_back(circle);
}

u32 get_num_circles()
{
  return (u32)circle_buffer.size();
}

std::vector<Athi_Circle> &get_circle_buffer()
{
  return circle_buffer;
}
