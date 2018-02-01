#pragma once

#include <string>
#include "athi_settings.h"  // console

#define GLEW_STATIC
#include <GL/glew.h>

#define check_gl_error() _check_gl_error(__FILE__, __LINE__)

static void _check_gl_error(const char *file, int line) {
  if constexpr (ONLY_RUNS_IN_DEBUG_MODE) {
    GLenum err(glGetError());

    while (err != GL_NO_ERROR) {
      std::string error;

      switch (err) {
        case GL_INVALID_OPERATION:
          error = "INVALID_OPERATION";
          break;
        case GL_INVALID_ENUM:
          error = "INVALID_ENUM";
          break;
        case GL_INVALID_VALUE:
          error = "INVALID_VALUE";
          break;
        case GL_OUT_OF_MEMORY:
          error = "OUT_OF_MEMORY";
          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
          error = "INVALID_FRAMEBUFFER_OPERATION";
          break;
      }
      console->warn("GL_{} - {}: {}", error, file, line);
      err = glGetError();
    }
  }
}