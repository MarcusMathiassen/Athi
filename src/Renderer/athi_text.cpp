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

#include "athi_text.h"
#include "../Utility/threadsafe_container.h" // ThreadSafe
#include "athi_camera.h" // cmaera
#include "../Utility/profiler.h" // gpu_profile, cpu_profile
#include "athi_renderer.h" // Renderer
#include "../athi_settings.h" // console

#include <algorithm> // std::find
#include <unordered_map> // std::unordered_map
#include <functional> // std::hash_


#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H


struct Font
{
    struct Character {
        s32 x0, y0, x1, y1;       // Texture coordinates
        glm::ivec2 size;          // Size of glyph
        glm::ivec2 bearing;       // Offset from baseline to left/top of glyph
        u32 advance;              // Horizontal offset to advance to next glyph
    };

    u32 texture_atlas;      // Handle to the texture atlas
    string name;            // Fonts name
    s32 size;               // Loaded size
    std::unordered_map<char, Character> characters; // a map of loaded chars. ['a'] gives you 'a'
};

struct Text
{
    string txt; // the actual string to render

    vec2 pos;   // position
    vec4 color; // text color
    f32 scale;  // desired scale

    string font;   // Desired font
};


static FT_Library ft;
#define NUM_GLYPHS 128

static const string default_path = "../Resources/Fonts/";

static Renderer renderer;

static vector<Text> texts;
static std::unordered_map<string, Font> font_library;

static vector<vec4> vertices;
static vector<vec4> colors;

void shutdown() noexcept
{
    FT_Done_FreeType(ft);
}
void init_text_renderer() noexcept
{
    auto &shader = renderer.make_shader();

    shader.sources = {
      "basic_text.vert",
      "basic_text.frag",
    };

    shader.uniforms = {
      "tex",
      "ortho_projection",
    };

    shader.attribs = {
      "vertices",
      "color",
    };

    auto &vertices_buffer = renderer.make_buffer("vertices");
    vertices_buffer.data_members = 4;
    vertices_buffer.divisor = 4;

    auto &colors_buffer = renderer.make_buffer("colors");
    colors_buffer.data_members = 4;
    colors_buffer.divisor = 1;

    constexpr u16 indices[6] = {0, 1, 2, 0, 2, 3};
    auto &indices_buffer = renderer.make_buffer("indices");
    indices_buffer.data = (void*)indices;
    indices_buffer.data_size = sizeof(indices);
    indices_buffer.type = buffer_type::element_array;

    renderer.finish();

    if (FT_Init_FreeType(&ft))
        console->error("Freetype: Could not init FreeType Library");
}

string load_font(const string& font_name, s32 size) noexcept
{
    // our new Font
    Font font;
    font.name = font_name;
    font.size = size;

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, (default_path+font_name).c_str(), 0, &face))
        console->error("Freetype: Failed to load font");

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, size);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load texture atlas for this font
    // quick and dirty max texture size estimate
    s32 max_dim = (1 + (face->size->metrics.height >> 6)) * ceilf(sqrtf(NUM_GLYPHS));
    s32 tex_width = 1;
    while(tex_width < max_dim) tex_width <<= 1;
    s32 tex_height = tex_width;

    // render glyphs to atlas
    char* pixels = (char*)calloc(tex_width * tex_height, 1);
    s32 pen_x = 0, pen_y = 0;

    for(s32 i = 0; i < NUM_GLYPHS; ++i)
    {
        FT_Load_Char(face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
        FT_Bitmap* bmp = &face->glyph->bitmap;

        if(pen_x + bmp->width >= tex_width)
        {
            pen_x = 0;
            pen_y += ((face->size->metrics.height >> 6) + 1);
        }

        for(s32 row = 0; row < bmp->rows; ++row)
        {
            for(s32 col = 0; col < bmp->width; ++col)
            {
                s32 x = pen_x + col;
                s32 y = pen_y + row;
                pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];
            }
        }

        font.characters[i].x0 = pen_x;
        font.characters[i].y0 = pen_y;
        font.characters[i].x1 = pen_x + bmp->width;
        font.characters[i].y1 = pen_y + bmp->rows;

        font.characters[i].bearing.x = face->glyph->bitmap_left;
        font.characters[i].bearing.y = face->glyph->bitmap_top;
        font.characters[i].advance = face->glyph->advance.x >> 6;

        pen_x += bmp->width + 1;
    }

    // Generate texture
    glGenTextures(1, &font.texture_atlas);
    glBindTexture(GL_TEXTURE_2D, font.texture_atlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_width, tex_height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);

    free(pixels);

    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Destroy FreeType once we're finished
    FT_Done_Face(face);

    // Add to the pile
    font_library.insert(std::pair<string, Font>(font_name, font));

    // Give the id back to the user
    return font_name;
}


void text_cpu_update_buffer() noexcept
{
    if (texts.empty()) return;

    cpu_profile p("text_cpu_update_buffer");


    colors.resize(texts.size());

    // For each text..
    for (auto text: texts)
    {
        f32 nx = text.pos.x;

        // Get the font
        Font font;
        if (font_library.find(text.font) != font_library.end())
        {
            font = font_library.at(text.font);
        }

        // Set the text color
        colors.emplace_back(text.color);

        // For each character in the text..
        for (auto c: text.txt)
        {
            const auto ch = font.characters[c];

            const f32 xpos = nx + ch.bearing.x * text.scale;
            const f32 ypos = text.pos.y - (ch.size.y - ch.bearing.y) * text.scale;

            const f32 w = ch.size.x * text.scale;
            const f32 h = ch.size.y * text.scale;

            // vec2 positions, vec2 texcoords
            vertices.emplace_back(xpos,      ypos + h, ch.x0, ch.y1);
            vertices.emplace_back(xpos,      ypos    , ch.x0, ch.y0);
            vertices.emplace_back(xpos + w,  ypos    , ch.x1, ch.y0);
            vertices.emplace_back(xpos + w,  ypos + h, ch.x1, ch.y1);

            // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            nx += (ch.advance >> 6) * text.scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }
    }
}

void text_gpu_update_buffer() noexcept
{
    if (texts.empty()) return;
    // Upload the buffers to the GPU
    gpu_profile p("text_gpu_update_buffer");
    renderer.update_buffer("vertices", vertices);
    renderer.update_buffer("colors",    colors);
}

void render_text() noexcept
{
    if (texts.empty()) return;

    renderer.bind();

    const auto proj = camera.get_ortho_projection();

    for (auto text: texts)
    {
        // Get the font
        auto &font = font_library.at(text.font);

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, font.texture_atlas);

        renderer.shader.set_uniform("ortho_projection", proj);
        renderer.shader.set_uniform("tex", font.texture_atlas);

        {
            CommandBuffer cmd;
            cmd.type = primitive::triangles;
            cmd.count = 6;
            cmd.has_indices = true;
            cmd.primitive_count = text.txt.size();

            gpu_profile p("text::draw");
            renderer.draw(cmd);
        }
    }
    texts.clear();
}

// Draws text using texture atlas
void draw_text(const string& font, const string& text, f32 x, f32 y, f32 scale, const vec4 &color) noexcept
{
    Text t;

    // If the font does not already exist, load it
    if (font_library.find(font) != font_library.end())
    {
        t.font = font;
    } else {
        t.font = load_font(font, scale);
    }

    t.txt = text;
    t.pos = {x,y};
    t.scale = scale;
    t.color = color;

    texts.emplace_back(t);
}

void draw_text(const string& font, const string& text, const vec2& pos, f32 scale, const vec4 &color) noexcept
{
    Text t;

    // If the font does not already exist, load it
    if (font_library.find(font) != font_library.end())
    {
        t.font = font;
    } else {
        t.font = load_font(font, scale);
    }

    t.txt = text;
    t.pos = pos;
    t.scale = scale;
    t.color = color;

    texts.emplace_back(t);
}
