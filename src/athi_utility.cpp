#include "athi_utility.h"

#include "./Renderer/athi_renderer.h"     // Renderer
#include "./Renderer/athi_camera.h"     // camera,
#include "./Renderer/athi_shader.h"     // Shader
#include "athi_transform.h"  // Transform

#include <algorithm>  // std::swap

f32 rand_f32(f32 min, f32 max) noexcept {
  return ((f32(rand()) / f32(RAND_MAX)) * (max - min)) + min;
}
vec2 rand_vec2(f32 min, f32 max) noexcept {
  return vec2(rand_f32(min, max), rand_f32(min, max));
}
vec3 rand_vec3(f32 min, f32 max) noexcept {
  return vec3(rand_f32(min, max), rand_f32(min, max), rand_f32(min, max));
}
vec4 rand_vec4(f32 min, f32 max) noexcept {
  return vec4(rand_f32(min, max), rand_f32(min, max), rand_f32(min, max),
              rand_f32(min, max));
}

vec4 rgb_to_hsv(vec4 in) noexcept {
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

vec4 hsv_to_rgb(s32 h, f32 s, f32 v, f32 a) noexcept {
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

  switch (i) {
    case 0:
      r = v;
      g = t;
      b = p;
      break;
    case 1:
      r = q;
      g = v;
      b = p;
      break;
    case 2:
      r = p;
      g = v;
      b = t;
      break;
    case 3:
      r = p;
      g = q;
      b = v;
      break;
    case 4:
      r = t;
      g = p;
      b = v;
      break;
    case 5:
    default:
      r = v;
      g = p;
      b = q;
      break;
  }
  return vec4(r, g, b, a);
}

vec4 lerp_hsv(vec4 a, vec4 b, f32 t) noexcept {
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

vec4 color_by_acceleration(const vec4 &min_color, const vec4 &max_color,
                           const vec2 &acc) noexcept {
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

std::unordered_map<string, f64> time_taken_by;

vec4 get_universal_current_color() {
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
void read_file(const char *file, char **buffer) noexcept {
  string buff, line;
  std::ifstream fileIn(file);
  while (std::getline(fileIn, line)) buff += line + '\n';
  *buffer = (char *)malloc((buff.length() + 1) * sizeof(char));
  strcpy(*buffer, buff.c_str());
}

void limit_FPS(u32 desired_framerate, f64 time_start_frame) noexcept {
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

vec2 to_view_space(vec2 v) noexcept {
  s32 width, height;
  glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
  v.x = -1.0f + 2 * v.x / width;
  v.y = 1.0f - 2 * v.y / height;
  return v;
}

static Renderer renderer;
void setup_fullscreen_quad() {

  u16 indices[6] = {0, 1, 2, 0, 2, 3};
  f32 positions[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
  f32 texcoords[] = {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};
  f32 colors[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                  1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

  auto &shader = renderer.make_shader();
  shader.sources = {"athi_fullscreen_quad.vert", "athi_fullscreen_quad.frag"};
  shader.attribs = {"position", "texcoord", "color"};
  shader.uniforms = {"transform", "res", "tex", "dir"};

  auto &vertex_buffer = renderer.make_buffer("positions");
  vertex_buffer.data = &positions[0];
  vertex_buffer.data_size = sizeof(positions);
  vertex_buffer.data_members = 2;
  vertex_buffer.stride = sizeof(f32) * 2;

  auto &texcoords_buffer = renderer.make_buffer("texcoords");
  texcoords_buffer.data = &texcoords[0];
  texcoords_buffer.data_size = sizeof(texcoords);
  texcoords_buffer.data_members = 2;
  texcoords_buffer.stride = sizeof(f32) * 2;

  auto &colors_buffer = renderer.make_buffer("colors");
  colors_buffer.data = &colors[0];
  colors_buffer.data_size = sizeof(colors);
  colors_buffer.data_members = 4;
  colors_buffer.stride = sizeof(f32) * 4;

  auto &indices_buffer = renderer.make_buffer("indices");
  colors_buffer.data = &indices[0];
  colors_buffer.data_size = sizeof(indices);
  colors_buffer.type = buffer_type::element_array;

  renderer.finish();
}

void draw_fullscreen_quad(u32 texture, const vec2 &dir) {

  CommandBuffer cmd;
  cmd.type = primitive::triangles;
  cmd.count = 6;
  cmd.has_indices = true;

  renderer.bind();

  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, texture);

  const auto proj = camera.get_ortho_projection();
  mat4 trans = proj * Transform().get_model();

  renderer.shader.set_uniform("transform", trans);
  renderer.shader.set_uniform("res", screen_width, screen_height);
  renderer.shader.set_uniform("tex", 0);
  renderer.shader.set_uniform("dir", dir);

  renderer.draw(cmd);
}

string get_cpu_brand() {
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
