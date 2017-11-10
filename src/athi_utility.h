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
#elif __APPLE__ || __linux__
#include <dispatch/dispatch.h>
  #include <sys/sysctl.h>
  #include <sys/types.h>
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
#ifdef __APPLE__
u32 get_cpu_freq();
u32 get_cpu_cores();
u32 get_cpu_threads();
std::string get_cpu_brand();
#endif
glm::vec4 get_universal_current_color();

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
