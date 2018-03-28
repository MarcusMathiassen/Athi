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

#include <GL/glew.h>

#include "../athi_typedefs.h"

// @Todo: These are not all the available enums, add more.
enum texture_filter {
  nearest = GL_NEAREST,
  linear = GL_LINEAR,
};

struct Fbo {
  u32 resolution[2];
  texture_filter fiter;
};

struct FrameBuffer
{
  s32  width, height;
  u32 fbo;
  u32 texture;

  FrameBuffer(u32 num_textures = 1, s32 width = 0, s32 height = 0);
  ~FrameBuffer();

  void resize(s32 width, s32 height) noexcept;
  void bind() const noexcept;
  void unbind() const noexcept;
  void clear() const noexcept;
};

extern vector<FrameBuffer> framebuffers;
