#include "athi_utility.h"
#include "athi_camera.h"
#include "athi_transform.h"
#include "athi_shader.h"

#include <vector>

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
    if (time_to_sleep > 2.0) // because of the inconsistent wakeup times from sleep
    {
#ifdef _WIN32
      Sleep((DWORD)time_to_sleep);
#elif __APPLE__
      const timespec time_in_nanoseconds_to_sleep{0, (long)(time_to_sleep * 1e6)};
      nanosleep(&time_in_nanoseconds_to_sleep, NULL);
#endif
    }
    while (time_spent_frame < frametime) {
      time_spent_frame = (glfwGetTime() - time_start_frame) * 1000.0;
    }
  }
}

enum {
  POSITION_BUFFER,
  COLOR_BUFFER,
  TEXTCOORD_BUFFER,
  INDICES_BUFFER,
  NUM_BUFFERS
};
unsigned int shader_program;
unsigned int VAO;
unsigned int VBO[NUM_BUFFERS];

Shader fullscreen_shader;
void setup_fullscreen_quad() {
  enum { POSITION_ATTR_LOC, TEXTCOORD_ATTR_LOC, COLOR_ATTR_LOC };

  fullscreen_shader.init("Fullscreen shader");
  fullscreen_shader.load_from_file("../Resources/athi_fullscreen_quad.vs", ShaderType::Vertex);
  fullscreen_shader.load_from_file("../Resources/athi_fullscreen_quad.fs", ShaderType::Fragment);
  fullscreen_shader.bind_attrib("position");
  fullscreen_shader.bind_attrib("texcoord");
  fullscreen_shader.bind_attrib("color");
  fullscreen_shader.link();
  fullscreen_shader.add_uniform("transform");
  fullscreen_shader.add_uniform("res");
  fullscreen_shader.add_uniform("tex");


  glGenVertexArrays(1, &VAO);
  glGenBuffers(NUM_BUFFERS, VBO);

  const std::uint16_t indices[6] = {0, 1, 2, 0, 2, 3};
  // const GLfloat positions[] = {
  //     -1.0, 1.0f,
  //     1.0f, 1.0f,
  //     1.0f, -1.0f,
  //     -1.0f, -1.0f,
  // };

  const GLfloat positions[] = {
      0.0f, 1.0f,
      1.0f, 1.0f,
      1.0f, 0.0f,
      0.0f, 0.0f,
  };

  const GLfloat textcoords[] = {
      0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
  };

  const GLfloat colors[] = {
      1.0f, 1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f, 1.0f,
      1.0f, 1.0f, 1.0f, 1.0f,
  };

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO[POSITION_BUFFER]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(POSITION_ATTR_LOC);
  glVertexAttribPointer(POSITION_ATTR_LOC, 2, GL_FLOAT, GL_FALSE,
                        sizeof(GLfloat) * 2, (void *)0);

  glBindBuffer(GL_ARRAY_BUFFER, VBO[TEXTCOORD_BUFFER]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(textcoords), textcoords, GL_STATIC_DRAW);
  glEnableVertexAttribArray(TEXTCOORD_ATTR_LOC);
  glVertexAttribPointer(TEXTCOORD_ATTR_LOC, 2, GL_FLOAT, GL_FALSE,
                        sizeof(GLfloat) * 2, (void *)0);

  glBindBuffer(GL_ARRAY_BUFFER, VBO[COLOR_BUFFER]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
  glEnableVertexAttribArray(COLOR_ATTR_LOC);
  glVertexAttribPointer(COLOR_ATTR_LOC, 4, GL_FLOAT, GL_FALSE,
                        sizeof(GLfloat) * 4, (void *)0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[INDICES_BUFFER]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
}

void draw_fullscreen_quad(std::uint32_t texture) {

  glBindVertexArray(VAO);
  fullscreen_shader.bind();
  glActiveTexture(GL_TEXTURE0 + 0);
  glBindTexture(GL_TEXTURE_2D, texture);

  const auto proj = camera.get_ortho_projection();
  mat4 trans = proj *  Transform().get_model();

  fullscreen_shader.setUniform("transform", trans);
  fullscreen_shader.setUniform("res", screen_width, screen_height);
  fullscreen_shader.setUniform("tex", 0);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
}

std::string get_cpu_brand() {
#ifdef _WIN32
  int CPUInfo[4] = {-1};
  unsigned nExIds, i = 0;
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
  return std::string(CPUBrandString);
#else
  char buffer[128];
  size_t bufferlen = 128;

  sysctlbyname("machdep.cpu.brand_string", &buffer, &bufferlen, NULL, 0);
  return string(buffer);
#endif
}
