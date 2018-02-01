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
#include "athi_typedefs.h"

#include "athi_transform.h"   // Transform
#include "./Renderer/athi_renderer.h"      // Shader

#define GLEW_STATIC
#include <GL/glew.h>

struct Athi_Line {
  vec2 p1, p2;
  f32 width;
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Transform transform;
  Athi_Line() = default;
};

struct Athi_Line_Manager {
  Renderer renderer;

  Athi_Line_Manager() = default;
  void init();
  void update();
  void draw();
};

void init_line_manager();
void add_line(Athi_Line *line);
void init_line_manager();
void draw_lines();

void draw_line(const vec2 &p1, const vec2 &p2, f32 width, const vec4 &color);

extern std::vector<Athi_Line> line_immediate_buffer;
extern std::vector<Athi_Line *> line_buffer;
extern Athi_Line_Manager athi_line_manager;
