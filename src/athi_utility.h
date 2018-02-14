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


#pragma once

#include "athi_typedefs.h"

#include "athi_settings.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>  // glfwGetTime

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

#include <cstdlib>  // rand

/* FOREGROUND */
#define RST "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

// Random number functions
f32 rand_f32(f32 min, f32 max) noexcept;
vec2 rand_vec2(f32 min, f32 max) noexcept;
vec3 rand_vec3(f32 min, f32 max) noexcept;
vec4 rand_vec4(f32 min, f32 max) noexcept;


std::tuple<size_t,size_t> get_begin_and_end(s32 i, s32 total, s32 threads) noexcept;
static bool file_exists(const std::string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}


// Color functions
vec4 hsv_to_rgb(s32 h, f32 s, f32 v, f32 a) noexcept;
vec4 rgb_to_hsv(vec4 in) noexcept;
vec4 lerp_hsv(vec4 a, vec4 b, f32 t) noexcept;
vec4 color_by_acceleration(const vec4 &min_color, const vec4 &max_color, const vec2 &acc) noexcept;
vec4 get_universal_current_color();
vec4 color_over_time(f64 time) noexcept;


// File functions
u64 GetFileTimestamp(const string& filename) noexcept;
void read_file(const char *file, char **buffer) noexcept;
string get_content_of_file(const string& file) noexcept;


// String functions
bool string_has(const string& str, char delim);
string remove_quotes(const string& str) noexcept;
string add_quotes(const string& str) noexcept;
vector<string> split_string(const string& str, char delim) noexcept;
string eat_chars(const string& str, vector<char> delims) noexcept;


vec2 to_view_space(vec2 v) noexcept;

void limit_FPS(u32 desired_framerate, f64 time_start_frame) noexcept;
string get_cpu_brand();


extern std::unordered_map<string, f64> time_taken_by;
extern std::vector<std::tuple<string, f64>> profiler_physics;

class physics_profile
{
private:
  f64     m_start_time{0.0};
  string  m_id;
public:
  physics_profile(const char* id) noexcept : m_id(id)
  {
    if constexpr (DEBUG_MODE)
      m_start_time = glfwGetTime();
  }
  ~physics_profile() noexcept
  {
    if constexpr (DEBUG_MODE)
      profiler_physics.emplace_back(std::tuple<string,f64>(m_id, ((glfwGetTime() - m_start_time) * 1000.0)));
  }
};

class profile {
 private:
  f64 start{0.0};
  string id;

 public:
  profile(const char *id_) noexcept {
    if constexpr (DEBUG_MODE) {
      id = id_;
      start = glfwGetTime();
    }
  }
  ~profile() noexcept {
    if constexpr (DEBUG_MODE) {
      time_taken_by[id] = (glfwGetTime() - start) * 1000.0;
    }
  }
};

template <class T, size_t S>
class Smooth_Average {
 public:
  Smooth_Average(T *var) : var(var) {}
  void add_new_frametime(T newtick) {
    tick_sum -= tick_list[tick_index];
    tick_sum += newtick;
    tick_list[tick_index] = newtick;
    if (++tick_index == S) tick_index = 0;
    *var = (static_cast<T>(tick_sum) / S);
  }

 private:
  T *var;
  size_t tick_index{0};
  T tick_sum{0};
  T tick_list[S];
};
