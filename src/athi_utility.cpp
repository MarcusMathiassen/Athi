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


#include "athi_utility.h"

#include "./Renderer/athi_camera.h"    // camera,
#include "./Renderer/athi_renderer.h"  // Renderer
#include "./Renderer/athi_shader.h"    // Shader
#include "athi_transform.h"            // Transform

#include <algorithm>  // std::swap
#include <mutex>  // std::mutex


#ifdef _WIN32
#include <sys/stat.h>
#else
// Not Windows? Assume unix-like.
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

f32  rand_f32 (f32 min, f32 max) noexcept { return ((f32(rand()) / f32(RAND_MAX)) * (max - min)) + min; }
vec2 rand_vec2(f32 min, f32 max) noexcept { return vec2(rand_f32(min, max), rand_f32(min, max)); }
vec3 rand_vec3(f32 min, f32 max) noexcept { return vec3(rand_f32(min, max), rand_f32(min, max), rand_f32(min, max)); }
vec4 rand_vec4(f32 min, f32 max) noexcept { return vec4(rand_f32(min, max), rand_f32(min, max), rand_f32(min, max), rand_f32(min, max)); }


f64 get_time() noexcept
{
  return glfwGetTime();
}

vec4 rgb_to_hsv(vec4 in) noexcept
{
  vec4 out;
  f64 min{0.0f}, max{0.0f}, delta{0.0f};

  min = in.r < in.g ? in.r : in.g;
  min = min < in.b ? min : in.b;

  max = in.r > in.g ? in.r : in.g;
  max = max > in.b ? max : in.b;

  out.z = max;  // v

  out.a = in.a;

  delta = max - min;
  if (delta < 0.00001) {
    out.y = 0;
    out.x = 0;  // undefined, maybe nan?
    return out;
  }
  if (max > 0.0) {  // NOTE: if Max is == 0, this divide would cause a crash
    out.y = (delta / max);  // s
  } else {
    // if max is 0, then r = g = b = 0
    // s = 0, h is undefined
    out.y = 0.0;
    out.x = NAN;  // its now undefined
    return out;
  }
  if (in.r >= max)                  // > is bogus, just keeps compilor happy
    out.x = (in.g - in.b) / delta;  // between yellow & magenta
  else if (in.g >= max)
    out.x = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
  else
    out.x = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan
  out.x *= 60.0;                          // degrees
  if (out.x < 0.0) out.x += 360.0;
  return out;
}

vec4 hsv_to_rgb(s32 h, f32 s, f32 v, f32 a) noexcept
{
  // gray
  if (s == 0.0f) return vec4(v, v, v, a);

  h = (h >= 360) ? 0 : h;
  const f32 hue = h * 1.0f / 60.0f;

  const s32 i = hue;
  const f32 f = hue - i;
  const f32 p = v * (1.0f - s);
  const f32 q = v * (1.0f - s * f);
  const f32 t = v * (1.0f - s * (1.0f - f));

  f32 r{0.0f}, g{0.0f}, b{0.0f};

  switch (i)
  {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    case 5:
    default: r = v; g = p; b = q; break;
  }

  return vec4(r, g, b, a);
}

vec4 lerp_hsv(vec4 a, vec4 b, f32 t) noexcept
{
  // Hue interpolation
  f32 h{0.0f};
  f32 d = b.x - a.x;
  if (a.x > b.x) {
    std::swap(a.x, b.x);

    d = -d;
    t = 1 - t;
  }

  if (d > 0.5)  // 180deg
  {
    a.x = a.x + 1;                // 360deg
    h = (a.x + t * (b.x - a.x));  // 360deg
  }
  if (d <= 0.5)  // 180deg
  {
    h = a.x + t * d;
  }

  h = h > 360 ? 360 : h < 0 ? 0 : h;

  // Interpolates the rest
  return vec4(h,                      // H
              a.y + t * (b.y - a.y),  // S
              a.z + t * (b.z - a.z),  // V
              a.a + t * (b.a - a.a)   // A
  );
}

vec4 color_by_acceleration(const vec4 &min_color, const vec4 &max_color, const vec2 &acc) noexcept
{
  vec2 temp = acc;
  // Get the HSV equivalent
  temp.x *= color_by_velocity_threshold;
  temp.y *= color_by_velocity_threshold;
  f32 mg = glm::length(temp);
  if (mg > 1.0) mg = 1.0;

  const auto c1 = rgb_to_hsv(min_color);
  const auto c2 = rgb_to_hsv(max_color);

  const auto c3 = lerp_hsv(c1, c2, mg);
  return hsv_to_rgb(c3.x, c3.y, c3.z, c3.w);
}

vec4 get_universal_current_color()
{
  universal_color_picker++;
  if (universal_color_picker > 6) universal_color_picker = 0;
  switch (universal_color_picker) {
    case 0:
      return pastel_red;
      break;
    case 1:
      return pastel_gray;
      break;
    case 2:
      return pastel_green;
      break;
    case 3:
      return pastel_orange;
      break;
    case 4:
      return pastel_yellow;
      break;
    case 5:
      return pastel_pink;
      break;
    case 6:
      return pastel_blue;
      break;
  }
  return vec4();
}

void read_file(const char *file, char **buffer) noexcept
{
  string buff, line;
  std::ifstream fileIn(file);
  while (std::getline(fileIn, line)) buff += line + '\n';
  *buffer = (char *)malloc((buff.length() + 1) * sizeof(char));
  strcpy(*buffer, buff.c_str());
}

string get_content_of_file(const string& file) noexcept
{
  std::ifstream ifs(file);
  return string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
}

void limit_FPS(u32 desired_framerate, f64 time_start_frame) noexcept
{
  const f64 frametime = (1000.0 / desired_framerate);
  f64 time_spent_frame = (glfwGetTime() - time_start_frame) * 1000.0;
  const f64 time_to_sleep = (frametime - time_spent_frame) * 0.7;

  if (time_to_sleep > 0.0) {
    if (time_to_sleep >
        2.0)  // because of the inconsistent wakeup times from sleep
    {
#ifdef _WIN32
      Sleep((DWORD)time_to_sleep);
#elif __APPLE__
      const timespec time_in_nanoseconds_to_sleep{0,
                                                  (long)(time_to_sleep * 1e6)};
      nanosleep(&time_in_nanoseconds_to_sleep, NULL);
#endif
    }
    while (time_spent_frame < frametime) {
      time_spent_frame = (glfwGetTime() - time_start_frame) * 1000.0;
    }
  }
}

vec4 color_over_time(f64 time) noexcept
{
  time = abs(time) * 330;
  return hsv_to_rgb(time, 1.0, 1.0, 1.0);
}

vec2 to_view_space(vec2 v) noexcept
{
  v.x = -1.0f + 2 * v.x / framebuffer_width;
  v.y = 1.0f - 2 * v.y / framebuffer_height;
  v.y *= -1.0f;
  return v;
}

string get_cpu_brand()
{
#ifdef _WIN32
  s32 CPUInfo[4] = {-1};
  u32 nExIds, i = 0;
  char CPUBrandString[0x40];
  // Get the information associated with each extended ID.
  __cpuid(CPUInfo, 0x80000000);
  nExIds = CPUInfo[0];
  for (i = 0x80000000; i <= nExIds; ++i) {
    __cpuid(CPUInfo, i);
    // Interpret CPU brand string
    if (i == 0x80000002)
      memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
    else if (i == 0x80000003)
      memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
    else if (i == 0x80000004)
      memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
  }
  // string includes manufacturer, model and clockspeed
  return string(CPUBrandString);
#else
  char buffer[128];
  size_t bufferlen = 128;

  sysctlbyname("machdep.cpu.brand_string", &buffer, &bufferlen, NULL, 0);
  return string(buffer);
#endif
}

u64 GetFileTimestamp(const string& filename) noexcept {
  u64 timestamp = 0;

#ifdef _WIN32
  struct __stat64 stFileInfo;
  if (_stat64(filename.c_str(), &stFileInfo) == 0) {
    timestamp = stFileInfo.st_mtime;
  }
#else
  struct stat fileStat;

  if (stat(filename.c_str(), &fileStat) == -1) {
    perror(filename.c_str());
    return 0;
  }

#ifdef __APPLE__
  timestamp = fileStat.st_mtimespec.tv_sec;
#else
  timestamp = fileStat.st_mtime;
#endif
#endif

  return timestamp;
}


bool string_has(const string& str, char delim)
{
    return (str.find(delim) == std::string::npos) ? false : true;
}


string remove_quotes(const string& str) noexcept
{
    int quote_count = 0;
    string new_str;
    for (auto c: str)
        if (c != '"') {
            new_str += c;
        }

    return new_str;
}

string add_quotes(const string& str) noexcept
{
    return '"' + str + '"';
}

vector<string> split_string(const string& str, char delim) noexcept
{
    vector<string> strings;

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(delim, prev)) != std::string::npos)
    {
        strings.emplace_back(str.substr(prev, pos - prev));
        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.emplace_back(str.substr(prev));

    return strings;
}

string eat_chars(const string& str, vector<char> delims) noexcept
{
    auto is_char = [delims](char c)
    {
       for (char delim: delims)
            if (c == delim)  return true;
        return false;
    };

    string new_str;
    for(u32 i = 0; i < str.size(); ++i)
    {
        if (!is_char(str[i]))
            new_str += str[i];
    }
    return new_str;
}
