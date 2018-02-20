#pragma once

#include "../athi_typedefs.h"

void shutdown() noexcept;
void init_text_renderer() noexcept;

u32 load_font(const string &font_name, s32 size = 2) noexcept;

void text_cpu_update_buffer() noexcept;
void text_gpu_update_buffer() noexcept;
void render_text() noexcept;

void draw_text(const string& font, const string &text, f32 x, f32 y, f32 scale, const vec4 &color) noexcept;
void draw_text(const string& font, const string &text, const vec2& pos, f32 scale, const vec4 &color) noexcept;
void immidiate_draw_text(u32 font_id, const string &text, f32 x, f32 y, f32 scale, const vec4 &color) noexcept;
