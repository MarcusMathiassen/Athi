#pragma once

#include "../athi_typedefs.h"

#include "../athi_settings.h" // console

#include "stb_image.h" // stbi_load, stbi_image_free

#define GLEW_STATIC
#include <GL/glew.h>


struct Texture {
  u32 id{0};
  f32 filtering{GL_LINEAR};
  Texture(const char *file, f32 _filtering) : filtering(_filtering) {
    s32 width{0}, height{0}, num_comp{0};
    u8 *image_data = stbi_load(file, &width, &height, &num_comp, 4);
    if (NULL == image_data)
      console->info("Texture loading failed: {}", file);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (s32)_filtering);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image_data);

    console->info("Texture loaded: {}", file);
  }

  Texture() = default;

  void bind(u32 unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, id);
  }
};
