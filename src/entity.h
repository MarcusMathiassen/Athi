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
#include "./Renderer/athi_shader.h" // Shader
#include "./Renderer/athi_buffer.h" // Buffer
#include "athi_transform.h"         // Transform

class Entity
{
protected:
    Buffer    buffer;
    Shader    shader;
    vec4      color;
    Transform transform;
public:
    virtual void update(f32 dt) = delete;
    virtual void draw()   = delete;
};

struct EntityManager
{
    vector<Entity> entities;

    void update(f32 dt) noexcept
    {
        for (auto & entity: entites)
        {
            entity.update(f32 dt);
        }
    }

    void draw() const noexcept
    {
        for (const auto & entity: entities)
        {
            entity.draw();
        }
    }

    template <class T>
    static void add_entity(const T& entity) noexcept
    {
        entities.emplace_back(t);
    }
};

// Example entity
struct Triangle: public Entity
{
    vec2 pos, vel, acc;

    void update(f32 dt) noexcept
    {
        this->transform.pos.x = sinf(glfwGetTime());
    }

    void draw() const noexcept
    {
        set_shader(argb_shader_no_texture);
        set_buffer(triangle_buffer);
        draw_command(0, 3);
    }
};
