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

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H


struct Character {
  u32 texture_id;   // ID handle of the glyph texture
  glm::ivec2 size;    // Size of glyph
  glm::ivec2 bearing;  // Offset from baseline to left/top of glyph
  u32 advance;    // Horizontal offset to advance to next glyph
};

struct Font {
    std::string name;
    s32 size;
    std::map<char, Character> characters;
};

static vector<Font> fonts;

static const string default_path = "../Resources/Fonts/";

static FT_Library ft;
static Renderer renderer;

void shutdown() noexcept
{
    fonts.clear();
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
      "color",
      "tex",
      "ortho_projection",
    };


    auto &vertices_buffer = renderer.make_buffer("vertices");
    vertices_buffer.data_members = 4;
    vertices_buffer.stride = sizeof(f32) * 4;

    constexpr u16 indices[6] = {0, 1, 2, 0, 2, 3};
    auto &indices_buffer = renderer.make_buffer("indices");
    indices_buffer.data = (void*)indices;
    indices_buffer.data_size = sizeof(indices);
    indices_buffer.type = buffer_type::element_array;

    renderer.finish();

    if (FT_Init_FreeType(&ft))
        console->error("Freetype: Could not init FreeType Library");
}

u32 load_font(const string &font_name, s32 size) noexcept
{
    // our new Font
    Font font;

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, (default_path+font_name).c_str(), 0, &face))
        console->error("Freetype: Failed to load font");

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, size);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 127; c++)
    {
      // Load character glyph
      if (FT_Load_Char(face, c, FT_LOAD_RENDER))
      {
          console->error("Freetype: Failed to load Glyph from file: {}", font_name);
          continue;
      }
      // Generate texture
      u32 texture;
      glGenTextures(1, &texture);
      glBindTexture(GL_TEXTURE_2D, texture);
      glTexImage2D(
          GL_TEXTURE_2D,
          0,
          GL_RED,
          face->glyph->bitmap.width,
          face->glyph->bitmap.rows,
          0,
          GL_RED,
          GL_UNSIGNED_BYTE,
          face->glyph->bitmap.buffer
      );
      // Set texture options
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // Now store character for later use
      Character ch = {
          texture,
          glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
          glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
          static_cast<u32>(face->glyph->advance.x)
      };
      font.characters.insert(std::pair<char, Character>(c, ch));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Destroy FreeType once we're finished
    FT_Done_Face(face);

    const auto id = static_cast<u32>(fonts.size());

    // Add to the pile
    fonts.emplace_back(font);

    // Give the id back to the user
    return id;
}

void immidiate_draw_text(u32 font_id, const string& text,  f32 x, f32 y, f32 scale, const vec4 &color) noexcept
{
     // dont render 100% transparent text
    if (color.a < 0.005f) return;

    renderer.bind();

    const auto orth_proj =  camera.get_ortho_projection();
    renderer.shader.set_uniform("color", color);
    renderer.shader.set_uniform("ortho_projection", orth_proj);

    glActiveTexture(GL_TEXTURE0);

    auto font = fonts[font_id];

    auto nx = x;

    CommandBuffer cmd;
    cmd.count = 6;
    cmd.type = primitive::triangles;
    cmd.has_indices = true;

    for (auto c: text)
    {
         auto ch = font.characters[c];

        const f32 xpos = nx + ch.bearing.x * scale;
        const f32 ypos = y - (ch.size.y - ch.bearing.y) * scale;

        const f32 w = ch.size.x * scale;
        const f32 h = ch.size.y * scale;

        f32 vertices[16] = {
          xpos,     ypos + h,   0.0f, 0.0f,
          xpos,     ypos,       0.0f, 1.0f,
          xpos + w, ypos,       1.0f, 1.0f,
          xpos + w, ypos + h,   1.0f, 0.0f
        };

        glBindTexture(GL_TEXTURE_2D, ch.texture_id);

        renderer.update_buffer("vertices", &vertices[0], sizeof(f32) * 16);

        renderer.draw(cmd);

        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        nx += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
}
