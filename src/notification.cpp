#include "notification.h"

#include "./Renderer/athi_rect.h"

void Label::draw() noexcept
{
    if (has_background) {
        // immidiate_draw_rounded_rect({button_pos.x, button_pos.y}, button_width, button_height, button_color);
    }

    // immidiate_draw_text(my_font, text.str, button_pos.x, button_pos.y, 1.0f * button_height * 0.1/4.0, text.color);
}
