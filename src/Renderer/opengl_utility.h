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

#pragma once

#include <iostream> // std::cerr
#include <string> // std::string

#include <GL/glew.h>

#define check_gl_error() _check_gl_error(__FILE__, __LINE__)
#define check_framebuffer_gl_error() _check_framebuffer_gl_error(__FILE__, __LINE__)

static void _check_gl_error(const char *file, int line) {
#ifdef NDEBUG
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
    std::cerr << "GL_" << error << " - " << file << ":" << line << std::endl;
    err = glGetError();
  }
#endif
}

static void _check_framebuffer_gl_error(const char *file, int line) {
#ifdef NDEBUG
  GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  while (err != GL_FRAMEBUFFER_COMPLETE) {
    std::string error;

    switch (err) {
      case GL_FRAMEBUFFER_UNDEFINED:
        error = "FRAMEBUFFER_UNDEFINED";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        error = "FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        error = "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        error = "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        error = "FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;
      case GL_FRAMEBUFFER_UNSUPPORTED:
        error = "FRAMEBUFFER_UNSUPPORTED";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        error = "FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        error = "FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
        break;
    }
    std::cerr << "GL_" << error << " - " << file << ":" << line << std::endl;
    err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  }
#endif
}


