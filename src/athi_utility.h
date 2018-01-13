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
  #include <unistd.h>
  #include <linux/sysctl.h>
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
void limit_FPS(u32 desired_framerate, double time_start_frame);
void validateShader(const char *file, const char *type, u32 shader);
void validateShaderProgram(const char *name, u32 shaderProgram);
u32 createShader(const char *file, const GLenum type);
std::string get_cpu_brand();
glm::vec4 get_universal_current_color();

static glm::vec2 to_view_space(glm::vec2 v) {
  int32_t width, height;
  glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);
  v.x = -1.0f + 2 * v.x / width;
  v.y = 1.0f - 2 * v.y / height;

  return v;
}

struct FrameBuffer {
  std::int32_t width, height;
  std::uint32_t fbo;
  std::uint32_t texture;
  FrameBuffer(std::int32_t width = 0, std::int32_t height = 0, std::uint32_t texture = 0) : width(width), height(height), texture(texture) {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture, 0);

    std::uint32_t rboDepthStencil;
    glGenRenderbuffers(1, &rboDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, rboDepthStencil);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  ~FrameBuffer() {
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texture);
  }
  void resize(std::int32_t width, std::int32_t height) noexcept {

    glDeleteTextures(1, &texture);

    this->width = width;
    this->height = height;

    glGenTextures(1, &texture);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           texture, 0);

    std::uint32_t rboDepthStencil;
    glGenRenderbuffers(1, &rboDepthStencil);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, rboDepthStencil);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void set_texture(std::uint32_t tex) noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB8, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    texture = tex;
  }
  void bind() const noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  }
  void unbind() const noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  void clear() const noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
};

//
extern std::unordered_map<std::string, f64> time_taken_by;
struct profile {
  f64 start{0.0};
  std::string id;

  profile(const char* id_) : id(id_) {
    start = glfwGetTime();
  }
  ~profile() {
    time_taken_by[id] = (glfwGetTime() - start) * 1000.0;
  }
};
//

class Semaphore {
 public:
  Semaphore(int count_ = 0) : count(count_) {}

  inline void notify() {
    std::unique_lock<std::mutex> lock(mtx);
    count++;
    cv.notify_one();
  }

  inline void wait() {
    std::unique_lock<std::mutex> lock(mtx);

    while (count == 0) {
      cv.wait(lock);
    }
    count--;
  }

 private:
  std::mutex mtx;
  std::condition_variable cv;
  int count;
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
