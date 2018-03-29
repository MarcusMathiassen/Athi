#pragma once

#include "./Renderer/athi_line.h" // draw_line
#include "./src/athi_utility.h" // get_time,
#include "./src/entity.h" // Entity

#include <vector> // std::vector
#include <glm/vec2.hpp> // glm::vec2

template <class T>
struct Graph: public Entity
{

    double current_time{0.0};
    double last_time{0.0};
    // This is the thing we grab data from each update
    T* observer;
    T current_value;

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

        if (!observer) return;

        if ((current_time - last_time) * update_frequency >= 1) {
            last_time = current_time;
            current_value = *observer;
        } else {

        }
    }

    void draw() const noexcept override
    {
        if (!observer) return;
        draw_line({5,5}, {*observer, 5}, 1, {1,0,0,1});
        draw_line({5,10}, {current_value, 10}, 1, {0,0,0,1});
    }
};
