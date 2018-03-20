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

#include <array>

struct ParticleSystem : public Entity
{
    struct Particle
    {
        vec2 position;
        vec2 velocity;
        vec2 acceleration;

        void update(float dt)
        {

        }

        void draw()
        {

        }
    };

    std::vector<Particle> particles;

    void update(float dt) override
    {

    }

    void draw() override
    {

    }
};

struct Circle: public Entity
{
    static const int vertices_amount = 360;

    template <class T>
    constexpr void embros(T) const noexcept {}
    constexpr void embros(double) const noexcept = delete;

    vec4 color{1,1,1,1};
    std::array<vec2, vertices_amount> vertices; 

    Circle()
    {
        for (int i = 0; i < vertices_amount; ++i) {
            vertices[i] = { 
                cos(i * kPI * 2.0f / vertices_amount),
                sin(i * kPI * 2.0f / vertices_amount)  
            };
        }
    }

    void update(float deltaTime) override
    {
        position = {get_mouse_pos(), 0};
        color = color_over_time(get_time());
        // position.x += sinf(get_time()) * 100.0f;
    }

    void draw() override
    {
        draw_circle(position, 300.0f, color, false);
    }
};


int main()
{
    Athi_Core athi;
    athi.init();

    Circle circle;
    athi.entity_manager.add_entity(&circle);

    athi.start();
}

void Athi::update()
{

}

void Athi::draw()
{

}
