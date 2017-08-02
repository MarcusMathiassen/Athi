#pragma once

#include "athi_typedefs.h"
#include "athi_settings.h"


#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
  #include <windows.h>
#elif __APPLE__
  #include <ctime>
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/sysctl.h>
#endif

#include <sstream>
#include <cstring>
#include <fstream>

static void readFile(const char* file, char** buffer);
static void limit_FPS(u32 desired_framerate, f64 time_start_frame);
static void validateShader(const char *file, const char *type, u32 shader);
static void validateShaderProgram(const char* name, u32 shaderProgram);
static u32 createShader(const char *file, const GLenum type);
static u32 get_cpu_freq();
static u32 get_cpu_cores();
static u32 get_cpu_threads();
static string get_cpu_brand();
static void get_universal_current_color(vec4 &col);

static vec4 get_universal_current_color()
{
  if (universal_color_picker > 6) universal_color_picker = 0;
  switch (universal_color_picker)
  {
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

template <typename T>
struct Optimizer
{
  T *var;
  struct {f64 frametime; T val; } previous;
  Optimizer(T* var) : var(var) { previous.val = *var; previous.frametime = 100000.0; }
  void feed(T val, f64 frametime)
  {
    if (frametime < previous.frametime)
    {
      *var = previous.val;
      previous.val = val;
      previous.frametime = frametime;
    } else *var = previous.val;
  }
};


struct SMA
{
  SMA(f64* var) : var(var){}
  f64* var;
  #define SMA_SAMPLES 50
  u32 tickindex{0};
  f64 ticksum{0};
  f64 ticklist[SMA_SAMPLES];
  void add_new_frametime(f64 newtick)
  {
      ticksum -= ticklist[tickindex];
      ticksum += newtick;
      ticklist[tickindex] = newtick;
      if(++tickindex == SMA_SAMPLES) tickindex = 0;
      *var = ((f64)ticksum/SMA_SAMPLES);
  }
};

static void readFile(const char* file, char** buffer)
{
  string buff, line;
  std::ifstream fileIn(file);
  while (std::getline(fileIn, line)) buff += line+'\n';
  *buffer = (char*)malloc((buff.length()+1) * sizeof(char));
  strcpy(*buffer, buff.c_str());
}

static void limit_FPS(u32 desired_framerate, f64 time_start_frame)
{
  const f64 frametime = (f64)(1000.0/desired_framerate);
  f64 time_spent_frame{(glfwGetTime() - time_start_frame) * 1000.0};
  const f64 time_to_sleep{(frametime - time_spent_frame) * 0.7};

  if (time_to_sleep > 0.0)
  {
    if (time_to_sleep > 2.0)  // because of the inconsistent wakeup times from sleep
    {
    #ifdef _WIN32
      Sleep((DWORD)time_to_sleep);
    #elif __APPLE__
      nanosleep((const struct timespec[]){{0, static_cast<long>(time_to_sleep*1e6)}}, NULL);
    #endif
    }
    while (time_spent_frame < frametime)
    {
      time_spent_frame = (glfwGetTime() - time_start_frame) * 1000.0;
    }
  }

}

static void validateShader(const char *file, const char *type, u32 shader)
{
  char    infoLog[512] = {0};
  s32     success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
    printf("\nERROR::FILE %s\n", file);
    printf("ERROR::SHADER::%s::COMPILATION::FAILED\n\n%s", type, infoLog);
  }
}

static void validateShaderProgram(const char* name, u32 shaderProgram)
{
  char  infoLog[512] = {0};
  s32   success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
    printf("\nERROR::SHADER::PROGRAM::%s::LINKING::FAILED\n\n%s",name, infoLog);
  }
}

static u32 createShader(const char *file, const GLenum type)
{
  char *source = NULL;
  readFile(file, &source);

  u32 shader = glCreateShader(type);
  if (NULL != source)
  {
    glShaderSource(shader, 1, &source, NULL);
    free(source);
  }

  glCompileShader(shader);

  switch (type)
  {
    case GL_VERTEX_SHADER:    validateShader(file, "VERTEX",    shader); break;
    case GL_FRAGMENT_SHADER:  validateShader(file, "FRAGMENT",  shader); break;
    case GL_GEOMETRY_SHADER:  validateShader(file, "GEOMETRY",  shader); break;
  }
  printf("Shader loaded: %s\n", file);
  return shader;
}

static u32 get_cpu_freq()
{
  u64 num = 0;
  size_t size = sizeof(num);

  if (sysctlbyname("hw.cpufrequency", &num, &size, NULL, 0) < 0) {
    perror("sysctl");
  }
  return num;
}

static u32 get_cpu_cores()
{
  u64 num = 0;
  size_t size = sizeof(num);

  if (sysctlbyname("hw.physicalcpu_max", &num, &size, NULL, 0) < 0) {
    perror("sysctl");
  }
  return num;
}

static u32 get_cpu_threads()
{
  u64 num = 0;
  size_t size = sizeof(num);

  if (sysctlbyname("hw.logicalcpu_max", &num, &size, NULL, 0) < 0) {
    perror("sysctl");
  }
  return num;
}

static string get_cpu_brand()
{
  char buffer[128];
  size_t bufferlen = 128;

  sysctlbyname("machdep.cpu.brand_string", &buffer, &bufferlen, NULL, 0);

  return string(buffer);
}

