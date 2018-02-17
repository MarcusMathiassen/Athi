#pragma once


// compile with: -std=c++1z -I/usr/local/include/freetype2 -lfreetype -lglew -lglfw -framework OpenGL

#include <cassert>
#include <iostream>
#include <cctype>
#include <vector>
#include <array>
#include <unordered_map>
#include <string>

#include "athi_renderer.h" // Renderer
#include "../Utility/profiler.h" // gpu_profile, cpu_profile
#include "../athi_typedefs.h"
#include "athi_camera.h" // cmaera

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace font_renderer {

  static Renderer renderer;

  #define NUM_GLYPHS 128

  struct glyph_info {
    int x0, y0, x1, y1; // coords of glyph in the texture atlas
    int x_off, y_off;   // left & top bearing when rendering
    int advance;        // x advance when rendering
  } info[NUM_GLYPHS];

  struct Character {
      int x0, y0, x1, y1;       // Texture coordinates
      unsigned int TextureID;   // ID handle of the glyph texture
      glm::ivec2 size;          // Size of glyph
      glm::ivec2 bearing;       // Offset from baseline to left/top of glyph
      unsigned int advance;     // Horizontal offset to advance to next glyph
  };

  struct Font {
    unsigned int texture_id;
    std::string name;
    int size;
    std::unordered_map<char, Character> characters;
  };

  struct Text
  {
    unsigned int font;
    float scale;
    vec2 pos;
    vec4 color;
    std::string txt;
  };

  static std::vector<Text> texts;
  static FT_Library ft;
  enum {VERTEX, INDICES, NUM_BUFFERS};
  static std::vector<Font> fonts;
  static unsigned int vao, vbo[NUM_BUFFERS], shader_program, text_color_location;

  static void shutdown() {
    fonts.clear();
    FT_Done_FreeType(ft);
  }
  static void init() {

    // Init shaders, buffers, and freetype2
    shader_program = glCreateProgram();
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);

    auto &shader = renderer.make_shader();

    shader.sources = {
      "basic_text.vert",
      "basic_text.frag"
    };

    shader.uniforms = {
      "tex",
      "ortho_projection",
    };

    shader.attribs = {
      "position",
      "texcoord",
      "color",
    };

    auto &positions_buffer = renderer.make_buffer("positions");
    positions_buffer.data_members = 2;
    positions_buffer.divisor = 1;

    auto &texcoord_buffer = renderer.make_buffer("texcoords");
    texcoord_buffer.data_members = 2;
    texcoord_buffer.divisor = 1;

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
      std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
  }

  void load_texture_atlas(FT_Face& face, Font &font)
  {
    // quick and dirty max texture size estimate

    int max_dim = (1 + (face->size->metrics.height >> 6)) * ceilf(sqrtf(NUM_GLYPHS));
    int tex_width = 1;
    while(tex_width < max_dim) tex_width <<= 1;
    int tex_height = tex_width;

    // render glyphs to atlas
    char* pixels = (char*)calloc(tex_width * tex_height, 1);
    int pen_x = 0, pen_y = 0;

    for(int i = 0; i < NUM_GLYPHS; ++i)
    {
      FT_Load_Char(face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
      FT_Bitmap* bmp = &face->glyph->bitmap;

      if(pen_x + bmp->width >= tex_width){
        pen_x = 0;
        pen_y += ((face->size->metrics.height >> 6) + 1);
      }

      for(int row = 0; row < bmp->rows; ++row){
        for(int col = 0; col < bmp->width; ++col){
          int x = pen_x + col;
          int y = pen_y + row;
          pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];
        }
      }

      // this is stuff you'd need when rendering individual glyphs out of the atlas

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
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        tex_width,
        tex_height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        pixels
    );

    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    font.texture_id = texture;
  }

  unsigned int load_font(const std::string &path_to_font, int size)
  {
    // our new Font
    Font font;

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, path_to_font.c_str(), 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, size);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load texture atlas for this font
    load_texture_atlas(face, font);

    // Destroy FreeType once we're finished
    FT_Done_Face(face);

    const auto id = static_cast<unsigned int>(fonts.size());

    // Add to the pile
    fonts.emplace_back(font);

    // Give the id back to the user
    return id;
  }

  static std::vector<glm::vec2> positions;
  static std::vector<glm::vec2> texcoords;
  static std::vector<glm::vec2> colors;

  void text_cpu_update_buffer() noexcept
  {
    cpu_profile p("text_cpu_update_buffer");

    // float nx = x;

    // int i = 0;
    // int j = 0;
    // int k = 0;

    // colors.resize(text.size());

    // for (auto text: texts)
    // {
    //   colors[j++] = text.color;

    //   positions.resize(text.txt.size());
    //   texcoords.resize(text.txt.size());

    //   for (auto c: text.txt)
    //   {
    //     const Character ch = font.characters[c];

    //     const float xpos = nx + ch.bearing.x * scale;
    //     const float ypos = y - (ch.size.y - ch.bearing.y) * scale;

    //     const float w = ch.size.x * scale;
    //     const float h = ch.size.y * scale;

    //     positions[i++] = {xpos,      ypos + h};
    //     positions[i++] = {xpos,      ypos   };
    //     positions[i++] = {xpos + w,  ypos   };
    //     positions[i++] = {xpos + w,  ypos + h};

    //     texcoords[j++] = {ch.x0, ch.y1};
    //     texcoords[j++] = {ch.x0, ch.y0};
    //     texcoords[j++] = {ch.x1, ch.y0};
    //     texcoords[j++] = {ch.x1, ch.y1};

    //     // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
    //     nx += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    //   }
    // }
  }
  void text_gpu_update_buffer() noexcept
  {
    {
      // Upload the buffers to the GPU
      gpu_profile p("text_gpu_update_buffer");
      renderer.update_buffer("positions", &positions[0], sizeof(vec2) * positions.size());
      renderer.update_buffer("texcoords", &texcoords[0], sizeof(vec2) * texcoords.size());
      renderer.update_buffer("color",     &colors[0],    sizeof(vec4) * colors.size());
    }
  }

  void render_text() noexcept
  {
    // renderer.bind();

    // const auto proj = camera.get_ortho_projection();

    // for (auto text: texts)
    // {
    //   auto font = fonts[font_id];
    //   renderer.shader.set_uniform("ortho_projection", proj);
    //   renderer.shader.set_uniform("tex", font.texture_id);
    //   {
    //     CommandBuffer cmd;
    //     cmd.type = primitive::triangles;
    //     cmd.count = 6;
    //     cmd.has_indices = true;
    //     cmd.primitive_count = positions.size();

    //     renderer.shader.set_uniform("ortho_projection", font.texture_id);

    //     gpu_profile p("text::draw");
    //     renderer.draw(cmd);
    //   }
    // }
  }

  // Draws text using texture atlas
  void _draw_text(unsigned int font_id, const std::string &text, float x, float y, float scale, const glm::vec4 &color)
  {
    // dont render 100% transparent text
    if (color.a < 0.005f) return;

    Text t;
    t.font = font_id;
    t.txt = text;
    t.pos = {x, y};
    t.scale = scale;
    t.color = color;

    texts.emplace_back(t);
  }

  void draw_text(unsigned int font_id, const std::string &text, float x, float y, float scale, const glm::vec4 &color)
  {

  //   // dont render 100% transparent text
  //   if (color.a < 0.005f) return;

  //   render_call([font_id, text, x, y, scale, color] {
  //     gpu_profile p("draw_text");
  //     glUseProgram(shader_program);
  //     glUniform4f(text_color_location, color.r, color.g, color.b, color.a);
  //     glActiveTexture(GL_TEXTURE0);
  //     glBindVertexArray(vao);

  //     auto font = fonts[font_id];
  //     float nx = x;

  //     for (auto c = text.begin(); c != text.end(); ++c)
  //     {
  //       const Character ch = font.characters[*c];

  //       const float xpos = nx + ch.bearing.x * scale;
  //       const float ypos = y - (ch.size.y - ch.bearing.y) * scale;

  //       const float w = ch.size.x * scale;
  //       const float h = ch.size.y * scale;

  //       const float positions[16] = {
  //           xpos,     ypos + h,   0.0, 0.0,
  //           xpos,     ypos,       0.0, 1.0,
  //           xpos + w, ypos,       1.0, 1.0,
  //           xpos + w, ypos + h,   1.0, 0.0
  //       };

  //       glBindTexture(GL_TEXTURE_2D, ch.TextureID);

  //       glBindBuffer(GL_ARRAY_BUFFER, vbo[VERTEX]);
  //       glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);

  //       glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);

  //       // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
  //       nx += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
  //     }
  // });
  }

} // namespace font_renderer
