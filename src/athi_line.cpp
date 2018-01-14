#include "athi_line.h"
#include "athi_camera.h"
#include "athi_rect.h"

std::vector<Athi_Line> line_immediate_buffer;
std::vector<Athi_Line *> line_buffer;
Athi_Line_Manager athi_line_manager;

Athi_Line_Manager::~Athi_Line_Manager() { glDeleteVertexArrays(1, &VAO); }

void Athi_Line_Manager::init() {
  shader_program = glCreateProgram();
  const u32 vs = createShader("../Resources/athi_line_shader.vs", GL_VERTEX_SHADER);
  const u32 gs = createShader("../Resources/athi_line_shader.gs", GL_GEOMETRY_SHADER);
  const u32 fs = createShader("../Resources/athi_line_shader.fs", GL_FRAGMENT_SHADER);

  glAttachShader(shader_program, vs);
  glAttachShader(shader_program, gs);
  glAttachShader(shader_program, fs);

  glLinkProgram(shader_program);
  glValidateProgram(shader_program);
  validateShaderProgram("line_constructor", shader_program);

  glDetachShader(shader_program, vs);
  glDetachShader(shader_program, gs);
  glDetachShader(shader_program, fs);
  glDeleteShader(vs);
  glDeleteShader(gs);
  glDeleteShader(fs);

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  uniform[POSITIONS] = glGetUniformLocation(shader_program, "positions");
  uniform[COLOR] = glGetUniformLocation(shader_program, "color");
}

void Athi_Line_Manager::draw() {
  if (line_buffer.empty() && line_immediate_buffer.empty())
    return;
  profile p("Athi_Line_Manager::draw()");
  glBindVertexArray(VAO);
  glUseProgram(shader_program);

  // This is for future use when i finally get to fix the missing transforms for
  // lines.
  // const auto proj = camera.get_ortho_projection();

  for (const auto &line : line_buffer) {

    // auto temp = Transform()
    // mat4 trans = proj *  temp.get_model();
    // glUniformMatrix4fv(athi_rect_manager.uniform[athi_rect_manager.TRANSFORM],
    // 1,
    //                      GL_FALSE, &trans[0][0]);

    glUniform4f(uniform[COLOR], line->color.r, line->color.g, line->color.b, line->color.a);
    glUniform4f(uniform[POSITIONS], line->p1.x, line->p1.y, line->p2.x, line->p2.y);
    glDrawArrays(GL_LINES, 0, 2);
  }

  for (const auto &line : line_immediate_buffer) {
    glUniform4f(uniform[COLOR], line.color.r, line.color.g, line.color.b, line.color.a);
    glUniform4f(uniform[POSITIONS], line.p1.x, line.p1.y, line.p2.x, line.p2.y);
    glDrawArrays(GL_POINTS, 0, 2);
  }
  line_immediate_buffer.clear();
}

void init_line_manager() { athi_line_manager.init(); }

void add_line(Athi_Line *line) { line_buffer.emplace_back(line); }

void draw_lines() { athi_line_manager.draw(); }

void draw_line(const vec2 &p1, const vec2 &p2, f32 width, const vec4 &color) {
  Athi_Line line;
  line.p1 = p1;
  line.p2 = p2;
  line.width = width;
  line.color = color;

  line_immediate_buffer.emplace_back(line);
}
