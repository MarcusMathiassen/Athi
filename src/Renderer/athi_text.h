#pragma once

#include "../athi_typedefs.h"

void shutdown() noexcept;
void init_text_renderer() noexcept;

size_t load_font(const string &font_name, int size) noexcept;

void text_cpu_update_buffer() noexcept;
void text_gpu_update_buffer() noexcept;
void render_text() noexcept;

void draw_text(const string& font, const string &text, float x, float y, float scale, const vec4 &color) noexcept;
void draw_text(const string& font, const string &text, const vec2& pos, float scale, const vec4 &color) noexcept;
