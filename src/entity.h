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

#include <glm/vec3.hpp> // glm::vec3
#include <vector> // std::vector

struct Entity
{
    std::size_t  id  {0};

    glm::vec3    position    {0, 0, 0};
    glm::vec3    rotation    {0, 0, 0};
    glm::vec3    scale       {1, 1, 1};

    virtual ~Entity() = default;

    virtual void update(float) noexcept = 0;
    virtual void draw() const noexcept = 0;
};

struct EntityManager
{
    std::vector<Entity*>      entities;

    void init() noexcept
    {

    }

    void update(float dt) noexcept
    {
        for (auto& entity: entities)
        {
            entity->update(dt);
        }
    }

    void draw() const noexcept
    {
        for (const auto& entity: entities)
        {
            entity->draw();
        }
    }

    template <class T>
    void add_entity(T* entity) noexcept
    {
        // Give the entity a unique ID.
        entity->id = entities.size();

        // Add it to entities
        entities.emplace_back(entity);
    }
};
