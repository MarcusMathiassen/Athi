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


#include "athi.h"
#include "./Renderer/athi_circle.h"
#include "./Renderer/athi_renderer.h"
#include "./Renderer/athi_camera.h"
#include "./src/athi_transform.h"
#include "./src/athi_input.h"

#include "./src/graph.h" // Graph


/*
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertices.size(), static_cast<const GLvoid*>(vertices.data()), GL_STATIC_DRAW);

    const GLint position_loc = 0;
    const GLint color_loc = 1;
    const GLint uv_loc = 2;

    glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
    glVertexAttribPointer(color_loc,    4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 2 * sizeof(GLfloat));
    glVertexAttribPointer(uv_loc,       2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 6 * sizeof(GLfloat));

    glEnableVertexAttribArray(position_loc);
    glEnableVertexAttribArray(color_loc);
    glEnableVertexAttribArray(uv_loc);
*/

int main()
{
    Athi_Core athi;
    athi.init();

    Graph<int> fps_graph;
    fps_graph.update_frequency = 60;
    fps_graph.observe(&framerate);

    athi.entity_manager.add_entity(&fps_graph);

    athi.start();
}

void Athi::update()
{

}

void Athi::draw()
{

}
