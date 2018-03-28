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

#include "athi_framebuffer.h"

#include "../Utility/console.h" // console
#include "opengl_utility.h" //   check_gl_error();

FrameBuffer::FrameBuffer(u32 num_textures, s32 width, s32 height) : width(width), height(height)
{
    glGenFramebuffers(1, &fbo); check_gl_error();
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); check_gl_error();

    glGenTextures(1, &texture); check_gl_error();
    glBindTexture(GL_TEXTURE_2D, texture); check_gl_error();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); check_gl_error();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0); check_gl_error();

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      console->warn("Framebuffer not completed. {}", __LINE__);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); check_gl_error();

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      console->warn("Framebuffer not completed. {}", __LINE__);
    }
}
FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &fbo); check_gl_error();
    glDeleteTextures(1, &texture); check_gl_error();
}
void FrameBuffer::resize(s32 width, s32 height) noexcept
{
    glDeleteTextures(1, &texture); check_gl_error();

    this->width = width;
    this->height = height;

    glGenTextures(1, &texture); check_gl_error();
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); check_gl_error();
    glBindTexture(GL_TEXTURE_2D, texture); check_gl_error();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0); check_gl_error();

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      console->warn("Framebuffer not completed. {}", __LINE__);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); check_gl_error();

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      console->warn("Framebuffer not completed. {}", __LINE__);
    }
}
void FrameBuffer::set_texture(u32 tex) noexcept
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); check_gl_error();
    glBindTexture(GL_TEXTURE_2D, tex); check_gl_error();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB8, GL_UNSIGNED_BYTE, NULL); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); check_gl_error();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0); check_gl_error();
    glBindFramebuffer(GL_FRAMEBUFFER, 0); check_gl_error();
    texture = tex;
}
void FrameBuffer::bind() const noexcept { glBindFramebuffer(GL_FRAMEBUFFER, fbo); check_gl_error(); }
void FrameBuffer::unbind() const noexcept { glBindFramebuffer(GL_FRAMEBUFFER, 0); check_gl_error(); }
void FrameBuffer::clear() const noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); check_gl_error();
    glClear(GL_COLOR_BUFFER_BIT); check_gl_error();
    glBindFramebuffer(GL_FRAMEBUFFER, 0); check_gl_error();
}
