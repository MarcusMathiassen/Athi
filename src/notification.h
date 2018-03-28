#pragma once

#include <string>           // string
#include <glm/vec4.hpp>     // glm::vec4

struct Text
{
    std::string     str{"Default"};
    glm::vec4       color{1,1,1,1};
    int             size{12};
};

struct Label
{
    Text    text;

    bool    has_background{true};
    bool    is_floating{false};

    void draw() noexcept;
};

struct Notification
{
    Label       label;
    float       fade_in_time{0.1f};
    float       fade_out_time{1.0f};
};
