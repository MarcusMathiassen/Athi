#pragma once

#include "athi_typedefs.h"

#ifdef _WIN32
  #include <windows.h>
#elif __APPLE__
  #include <ctime>
  #include <unistd.h>
#endif

#include <sstream>
#include <cstring>
#include <fstream>

#include <sys/types.h>
#include <sys/sysctl.h>

static void readFile(const char* file, char** buffer);
static void limit_FPS(u32 desired_framerate, f64 time_start_frame);
static void validateShader(const char *file, const char *type, u32 shader);
static void validateShaderProgram(const char* name, u32 shaderProgram);
static u32 createShader(const char *file, const GLenum type);
static u64 get_cpu_freq();
static u64 get_cpu_cores();
static u64 get_cpu_threads();
static string get_cpu_brand();

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
  const f64 time_to_sleep{(frametime - time_spent_frame) * 0.8};
  if (time_to_sleep > 0.0)
  {
  #ifdef _WIN32
    Sleep((DWORD)time_to_sleep);
  #elif __APPLE__
    nanosleep((const struct timespec[]){{0, static_cast<long>(time_to_sleep*1e6)}}, NULL);
  #endif
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

static u64 get_cpu_freq()
{
  u64 num = 0;
  size_t size = sizeof(num);

  if (sysctlbyname("hw.cpufrequency", &num, &size, NULL, 0) < 0) {
    perror("sysctl");
  }
  return num;
}

static u64 get_cpu_cores()
{
  u64 num = 0;
  size_t size = sizeof(num);

  if (sysctlbyname("hw.physicalcpu_max", &num, &size, NULL, 0) < 0) {
    perror("sysctl");
  }
  return num;
}

static u64 get_cpu_threads()
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

