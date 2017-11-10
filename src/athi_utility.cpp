#include "athi_utility.h"


std::unordered_map<std::string, f64> time_taken_by;

vec4 get_universal_current_color() {
  if (universal_color_picker > 6)
    universal_color_picker = 0;
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
void read_file(const char *file, char **buffer) {
  string buff, line;
  std::ifstream fileIn(file);
  while (std::getline(fileIn, line))
    buff += line + '\n';
  *buffer = (char *)malloc((buff.length() + 1) * sizeof(char));
  strcpy(*buffer, buff.c_str());
}

void limit_FPS(u32 desired_framerate, f64 time_start_frame) {
  const f64 frametime = (1000.0 / desired_framerate);
  f64 time_spent_frame = (glfwGetTime() - time_start_frame) * 1000.0;
  const f64 time_to_sleep = (frametime - time_spent_frame) * 0.7;

  if (time_to_sleep > 0.0) {
    if (time_to_sleep >
        2.0) // because of the inconsistent wakeup times from sleep
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

void validateShader(const char *file, const char *type, u32 shader) {
  char infoLog[512] = {0};
  s32 success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
    printf("\nERROR::FILE %s\n", file);
    printf("ERROR::SHADER::%s::COMPILATION::FAILED\n\n%s", type, infoLog);
  }
}

void validateShaderProgram(const char *name, u32 shaderProgram) {
  char infoLog[512] = {0};
  s32 success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(shaderProgram, sizeof(infoLog), NULL, infoLog);
    printf("\nERROR::SHADER::PROGRAM::%s::LINKING::FAILED\n\n%s", name,
           infoLog);
  }
}

u32 createShader(const char *file, const GLenum type) {
  char *source = NULL;
  read_file(file, &source);

  u32 shader = glCreateShader(type);
  if (NULL != source) {
    glShaderSource(shader, 1, &source, NULL);
    free(source);
  }

  glCompileShader(shader);

  switch (type) {
  case GL_VERTEX_SHADER:
    validateShader(file, "VERTEX", shader);
    break;
  case GL_FRAGMENT_SHADER:
    validateShader(file, "FRAGMENT", shader);
    break;
  case GL_GEOMETRY_SHADER:
    validateShader(file, "GEOMETRY", shader);
    break;
  }
  return shader;
}
#ifdef __APPLE__ || __linux__
u32 get_cpu_freq() {
  u32 num = 0;
  size_t size = sizeof(num);

  if (sysctlbyname("hw.cpufrequency", &num, &size, NULL, 0) < 0) {
    perror("sysctl");
  }
  return num;
}

u32 get_cpu_cores() {
  u32 num = 0;
  size_t size = sizeof(num);

  if (sysctlbyname("hw.physicalcpu_max", &num, &size, NULL, 0) < 0) {
    perror("sysctl");
  }
  return num;
}

u32 get_cpu_threads() {
  u32 num = 0;
  size_t size = sizeof(num);

  if (sysctlbyname("hw.logicalcpu_max", &num, &size, NULL, 0) < 0) {
    perror("sysctl");
  }
  return num;
}

std::string get_cpu_brand() {
  char buffer[128];
  size_t bufferlen = 128;

  sysctlbyname("machdep.cpu.brand_string", &buffer, &bufferlen, NULL, 0);

  return string(buffer);
}
#endif
