#pragma once

#include "athi_typedefs.h"

#define GLEW_STATIC
#include <GL/glew.h>

#include <iostream>

class Athi_Resource_Manager
{
private:
  std::vector<u32> shader_program_buffer;

public:
  Athi_Resource_Manager() = default;
};

void load_shader(const char* path, GLenum type)
{

}
