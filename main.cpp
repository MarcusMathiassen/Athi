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

struct Rectangle: public Entity
{
    float radius{200.0f};
    vec4    color   {1,1,1,1};

    mat4    model;

    static Renderer *renderer;

    Rectangle()
    {
        if (!renderer) {
            renderer = new Renderer("Rectangle"); //&make_renderer("Rectangle");

            auto &shader = renderer->make_shader();

            shader.sources = {"argb_no_tex.vert", "argb_no_tex.frag"};
            shader.attribs = {"position"};
            shader.uniforms = {"transform", "color"};

            array<vec2, 4> vertices = {
                vec2(0.0f, 1.0f),
                vec2(1.0f, 1.0f),
                vec2(1.0f, 0.0f),
                vec2(0.0f, 0.0f),
            };

            auto &vbo = renderer->make_buffer("position");
            vbo.data = &vertices[0];
            vbo.data_size = vertices.size() * sizeof(vertices[0]);
            vbo.data_members = 2;
            vbo.type  = buffer_type::array_buffer;
            vbo.usage = buffer_usage::static_draw;

            constexpr u16 indices[6] = {0, 1, 2, 0, 2, 3};
            auto &indices_buffer = renderer->make_buffer("indices");
            indices_buffer.data = (void*)indices;
            indices_buffer.data_size = sizeof(indices);
            indices_buffer.type = buffer_type::element_array;

            renderer->finish();
        }
    }

    void update(float deltaTime) noexcept override
    {
        const auto current_time = get_time();

        position = {get_mouse_pos(), 0};
        position.x += sinf(current_time) * 100.0f;
        color = color_over_time(get_time());

        const auto proj = camera.get_ortho_projection();

        Transform temp;
        temp.pos = position;
        temp.scale = {radius*cos(current_time*0.1f), radius*sinf(current_time*0.1f), 1.0f};

        model = proj * temp.get_model();
    }

    void draw() const noexcept override
    {
        renderer->bind();

        renderer->shader.set_uniform("transform", model);
        renderer->shader.set_uniform("color", color);

        CommandBuffer cmd;
        cmd.type = primitive::triangles;
        cmd.count = 6;
        cmd.has_indices = true;

        renderer->draw(cmd);
    }
};
Renderer* Rectangle::renderer;


struct Circle: public Entity
{

    float   radius  {100.0f};
    vec4    color   {1,1,1,1};

    mat4    model;

    static const int vertices_amount = 360;
    static Renderer *renderer;

    Circle()
    {
        if (!renderer) {
            renderer = new Renderer("Circle"); // &make_renderer("Circle");

            auto &shader = renderer->make_shader();
            shader.sources = {"argb_no_tex.vert", "argb_no_tex.frag"};
            shader.attribs = {"position"};
            shader.uniforms = {"transform", "color"};

            array<vec2, vertices_amount> vertices;
            for (int i = 0; i < vertices_amount; ++i) {
              vertices[i] = {
                                cos(i * kPI * 2.0f / vertices_amount),
                                sin(i * kPI * 2.0f / vertices_amount)
                            };
            }

            auto &vbo = renderer->make_buffer("position");
            vbo.data = &vertices[0];
            vbo.data_size = vertices.size() * sizeof(vertices[0]);
            vbo.data_members = 2;
            vbo.type  = buffer_type::array_buffer;
            vbo.usage = buffer_usage::static_draw;

            renderer->finish();
        }
    }

    void update(float deltaTime) noexcept override
    {
        const auto current_time = get_time();

        position = {get_mouse_pos(), 0};
        position.x += sinf(current_time) * 100.0f;
        color = color_over_time(get_time());

        const auto proj = camera.get_ortho_projection();

        Transform temp;
        temp.pos = position;
        temp.scale = {radius*sinf(current_time*0.1f), radius*sinf(current_time*0.1f), 1.0f};

        model = proj * temp.get_model();
    }

    void draw() const noexcept override
    {
        renderer->bind();

        renderer->shader.set_uniform("transform", model);
        renderer->shader.set_uniform("color", color);

        CommandBuffer cmd;
        cmd.type = primitive::triangle_fan;
        cmd.count = vertices_amount;

        renderer->draw(cmd);
    }
};
Renderer* Circle::renderer;


int main()
{
    Athi_Core athi;
    athi.init();

    Rectangle rectangle;
    Circle circle;

    // athi.entity_manager.add_entity(&rectangle);
    // athi.entity_manager.add_entity(&circle);

    athi.start();
}

void Athi::update()
{

}

void Athi::draw()
{

}
