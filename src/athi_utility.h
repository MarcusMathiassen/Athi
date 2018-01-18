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
#ifdef __APPLE__
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

/* FOREGROUND */
#define RST  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

void read_file(const char *file, char **buffer);
void limit_FPS(std::uint32_t desired_framerate, double time_start_frame);
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

  profile(const char *id_) noexcept {
    if constexpr (ONLY_RUNS_IN_DEBUG_MODE) {
      id = id_;
      start = glfwGetTime();
    }
  }

  ~profile() noexcept {
    if constexpr (ONLY_RUNS_IN_DEBUG_MODE) {
      time_taken_by[id] = (glfwGetTime() - start) * 1000.0;
    }
  }
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
