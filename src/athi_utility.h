#pragma once

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>

#ifdef _WIN32
#include <windows.h>
#endif
#if __APPLE__
#include <sys/sysctl.h>
#include <sys/types.h>
#endif
#if __linux__
#include <linux/sysctl.h>
#include <unistd.h>
#endif

#ifndef _WIN32
#include <ctime>
#endif

#include <cstring>
#include <fstream>
#include <sstream>

#include <vector>

#include "athi_settings.h"
#include "athi_typedefs.h"

void read_file(const char *file, char **buffer);
void limit_FPS(std::uint32_t desired_framerate, double time_start_frame);
void validateShader(const char *file, const char *type, std::uint32_t shader);
void validateShaderProgram(const char *name, std::uint32_t shaderProgram);
std::uint32_t createShader(const char *file, const GLenum type);
std::string get_cpu_brand();
glm::vec4 get_universal_current_color();

static glm::vec2 to_view_space(glm::vec2 v) {
  int32_t width, height;
  glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
  v.x = -1.0f + 2 * v.x / width;
  v.y = 1.0f - 2 * v.y / height;

  return v;
}

void setup_fullscreen_quad();
void draw_fullscreen_quad(std::uint32_t texture);

extern std::unordered_map<std::string, double> time_taken_by;
struct profile {
  double start{0.0};
  std::string id;

  profile(const char *id_) : id(id_) { start = glfwGetTime(); }
  ~profile() { time_taken_by[id] = (glfwGetTime() - start) * 1000.0; }
};

template <class T, size_t S> class Smooth_Average {
public:
  Smooth_Average(T *var) : var(var) {}
  void add_new_frametime(T newtick) {
    tick_sum -= tick_list[tick_index];
    tick_sum += newtick;
    tick_list[tick_index] = newtick;
    if (++tick_index == S)
      tick_index = 0;
    *var = (static_cast<T>(tick_sum) / S);
  }

private:
  T *var;
  size_t tick_index{0};
  T tick_sum{0};
  T tick_list[S];
};
