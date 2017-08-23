#pragma once

#include "athi_settings.h"
#include "athi_typedefs.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <thread>        
#include <mutex>         
#include <condition_variable>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <ctime>
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

#include <cstring>
#include <fstream>
#include <sstream>

#include <vector>

void read_file(const char* file, char** buffer);
void limit_FPS(u32 desired_framerate, f64 time_start_frame);
void validateShader(const char *file, const char *type, u32 shader);
void validateShaderProgram(const char *name, u32 shaderProgram);
u32 createShader(const char *file, const GLenum type);
u32 get_cpu_freq();
u32 get_cpu_cores();
u32 get_cpu_threads();
string get_cpu_brand();
vec4 get_universal_current_color();

class Semaphore {
public:
    Semaphore (int count_ = 0) : count(count_) {}

    inline void notify()
    {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }

    inline void wait()
    {
        std::unique_lock<std::mutex> lock(mtx);

        while(count == 0){
            cv.wait(lock);
        }
        count--;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};


struct SMA {
  SMA(f64 *var) : var(var) {}
  f64 *var;
#define SMA_SAMPLES 50
  u32 tickindex{0};
  f64 ticksum{0};
  f64 ticklist[SMA_SAMPLES];
  void add_new_frametime(f64 newtick) {
    ticksum -= ticklist[tickindex];
    ticksum += newtick;
    ticklist[tickindex] = newtick;
    if (++tickindex == SMA_SAMPLES)
      tickindex = 0;
    *var = ((f64)ticksum / SMA_SAMPLES);
  }
};
