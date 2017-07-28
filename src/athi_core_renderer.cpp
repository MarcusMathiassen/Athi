#include "athi_core_renderer.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

void Athi_Core_Renderer::init()
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(0,0,0,1);
}

void Athi_Core_Renderer::update()
{
  glfwSwapInterval(vsync);
}

void Athi_Core_Renderer::UI()
{
  ImGui::SliderInt("limit fps", &framerate_limit, 0, 300);
  ImGui::Checkbox("vsync", &vsync);
  ImGui::Text("%dfps %.3fms",(int)(1000/frametime), frametime);
  ImGui::Separator();
}
