#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include <iostream>

#include "athi_camera.h"
#include "athi_texture.h"
#include "athi_transform.h"
#include "athi_typedefs.h"
#include "athi_utility.h"
#include <memory>
#include <vector>

#define TOP 0.92f
#define BOTTOM -1.0f
#define LEFT -1.0f
#define RIGHT 1.0f

#define ROW 0.06f
#define DIST_BETW_CHAR 0.06125f * 0.5f

#define FONT_ATLAS_CHARACTERS_ROWS 16
#define FONT_ATLAS_CHARACTERS_COLS 16
#define FONT_ATLAS_CHARACTER_WIDTH 0.06125f
#define FONT_ATLAS_CHARACTER_HEIGHT 0.06125f

void draw_all_text();
void init_text_manager();
void shutdown_text_manager();

struct Athi_Text {
  std::string str;
  Transform transform;
  vec2 pos{0.0f, 0.0f};
  vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
  Athi_Text() = default;
  void init();
};

extern std::vector<Athi_Text *> text_buffer;

void add_text(Athi_Text *text);
void draw_text(std::string str, vec2 pos, vec4 color);

struct Athi_Text_Manager {
  const u16 indices[6]{0, 1, 2, 0, 2, 3};

  enum { TRANSFORM, COLOR, TEXTCOORD_INDEX, NUM_UNIFORMS };
  enum { INDICES, NUM_BUFFERS };
  u32 VAO;
  u32 VBO[NUM_BUFFERS];
  u32 shader_program;
  u32 uniform[NUM_UNIFORMS];
  Texture texture;
  std::string font_atlas_path;

  Athi_Text_Manager() = default;
  ~Athi_Text_Manager();

  void update() {}
  void draw();
  void init();
};

static Athi_Text_Manager athi_text_manager;
