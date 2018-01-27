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

#include <cstdlib> // rand

#include <glm/vec2.hpp>

static float rand_f32(float min, float max) noexcept
{
    return ((float(rand()) / float(RAND_MAX)) * (max - min)) + min;
}

static glm::vec2 rand_vec2(float min, float max) noexcept {
  return glm::vec2(rand_f32(min, max), rand_f32(min, max));
}

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

struct HSV {
  float h{0.0f},s{0.0f},v{0.0f},a{1.0f};
  HSV() = default;
  HSV(float _h, float _s, float _v ,float _a) 
  : h(_h), s(_s), v(_v), a(_a){}
};

static HSV rgb_to_hsv(glm::vec4 in) {
  HSV         out;
  double      min, max, delta;

  min = in.r < in.g ? in.r : in.g;
  min = min  < in.b ? min  : in.b;

  max = in.r > in.g ? in.r : in.g;
  max = max  > in.b ? max  : in.b;

  out.v = max;                                // v
  delta = max - min;
  if (delta < 0.00001)
  {
      out.s = 0;
      out.h = 0; // undefined, maybe nan?
      return out;
  }
  if( max > 0.0 ) { // NOTE: if Max is == 0, this divide would cause a crash
      out.s = (delta / max);                  // s
  } else {
      // if max is 0, then r = g = b = 0              
      // s = 0, h is undefined
      out.s = 0.0;
      out.h = NAN;                            // its now undefined
      return out;
  }
  if( in.r >= max )                           // > is bogus, just keeps compilor happy
      out.h = ( in.g - in.b ) / delta;        // between yellow & magenta
  else
  if( in.g >= max )
      out.h = 2.0 + ( in.b - in.r ) / delta;  // between cyan & yellow
  else
      out.h = 4.0 + ( in.r - in.g ) / delta;  // between magenta & cyan

  out.h *= 60.0;                              // degrees

  if( out.h < 0.0 )
      out.h += 360.0;

  return out;
}


#ifdef _WIN32
  static glm::vec4 getHSV(u16 h, f32 s, f32 v, f32 a)
#else
  static constexpr glm::vec4 getHSV(u16 h, f32 s, f32 v, f32 a)
#endif
{
  h = (h >= 360) ? 0 : h;
  const f32 hue { (f32)h * 0.016666f };

  const u8  i   { (u8)hue };
  const f32 f   { hue - i };
  const f32 p   { v * (1.0f - s) };
  const f32 q   { v * (1.0f - s*f) };
  const f32 t   { v * (1.0f - s*( 1.0f-f )) };

  f32 r{0.0f}, g{0.0f}, b{0.0f};

  switch(i)
  {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5:
    default: r = v; g = p; b = q; break;
  }
  return glm::vec4(r,g,b,a);
}
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
void draw_fullscreen_quad(std::uint32_t texture, const glm::vec2& dir);

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
