#include "athi_spring.h"

vector<unique_ptr<Athi_Spring> > spring_buffer;

void add_spring(const Athi_Spring& s)
{
  spring_buffer.emplace_back(std::make_unique<Athi_Spring>(s));
}

void update_springs()
{
  for (auto &s: spring_buffer)
    s->update();
}

void draw_springs()
{
  for (auto &s: spring_buffer)
    s->draw();
}

void attach_spring(Athi_Circle &a, Athi_Circle &b)
{
  Athi_Spring spring;

  spring.object = &a;
  spring.anchor = &b;
  spring.length = glm::distance(a.pos, b.pos);

  add_spring(spring);
}
