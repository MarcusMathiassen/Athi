#pragma once

#include "athi_transform.h"
#include "athi_settings.h"
#include "athi_quadtree.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>

struct Particle {
  int32_t id;               
  glm::vec2 pos{0.0f, 0.0f}; 
  glm::vec2 vel{0.0f, 0.0f}; 
  glm::vec2 acc{0.0f, 0.0f}; 
  float mass;

  void update() {
    if (physics_gravity) vel.y -= 0.000981f * timestep;

    vel.x += (acc.x * timestep);
    vel.y += (acc.y * timestep);
    pos.x += (vel.x * timestep);
    pos.y += (vel.y * timestep);
    acc *= 0;
  }
};

struct ParticleManager {

  static constexpr int32_t num_verts{36};

  std::vector<Particle>   particles;   
  std::vector<float>      radii;
  std::vector<Transform>  transforms;   
  std::vector<glm::vec4>  colors;      
  std::vector<glm::mat4>  models;       

  Quadtree<Particle> quadtree = Quadtree<Particle>(quadtree_depth, quadtree_capacity, glm::vec2(-1, -1), glm::vec2(1, 1));

  enum { POSITION, COLOR, TRANSFORM, NUM_BUFFERS };
  uint32_t vao;
  uint32_t vbo[NUM_BUFFERS];
  uint32_t shader_program;
  size_t model_bytes_allocated{0};
  size_t color_bytes_allocated{0};

  void init();
  void update();
  void draw() const;
  bool collision_check(const Particle &a, const Particle &b) const;
  void collision_resolve(Particle &a, Particle &b);
  void separate(Particle &a, Particle &b);
  void collision_logNxN(size_t total, size_t begin, size_t end);
  void collision_quadtree(const std::vector<std::vector<int>> &cont, size_t begin, size_t end);
  void add(const glm::vec2 &pos, float radius, const glm::vec4 &color = glm::vec4(1,1,1,1));
  void erase_all();
};

extern ParticleManager particle_manager;
