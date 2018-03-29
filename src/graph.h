#pragma once

#include "Renderer/athi_line.h" // draw_line

#include <vector> // std::vector
#include <glm/vec2.hpp> // glm::vec2

template <class T>
struct Graph: public Entity
{
    double start_time {0.0};
    double current_time {0.0};
    double last_time{0.0};


    T current_value;
    // This is the thing we grab data from each update
    T* observer;

    int seconds_passed_since_last_update {0};

    int update_frequency {1};

    std::vector<glm::vec2> vertices;

    Graph() = default;

    void observe(T* variable) noexcept {
        observer = variable;
        current_value = *variable;
    }

    void update(float dt) noexcept override
    {
        current_time = get_time();
        if (current_time - last_time == update_frequency)
        {
            last_time = current_time;
            // Time to update.
            current_value = *observer;

        } else {

            // Not yet there, wait more.
        }
    }

    void draw() const noexcept override
    {
        draw_line({0,0}, {*observer, 0}, 1, {1,0,0,1});
    }
};
