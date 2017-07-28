#include "athi_circle.h"

void Athi_Circle::update()
{

}

void Athi_Circle::draw()
{

}

void Athi_Circle::borderCollision()
{
  if (pos.x <= -1.0f + radius && vel.x < 0.0f)
  {
    pos.x = -1.0f + radius;
    vel.x = -vel.x;
  }
  if (pos.x >= 1.0f - radius && vel.x > 0.0f)
  {
    pos.x = (1.0f - radius);
    vel.x = -vel.x;
  }
  if (pos.y <= -1.0f + radius && vel.y < 0.0f)
  {
    pos.y = -1.0f + radius;
    vel.y = -vel.y;
  }
  if (pos.y >= 1.0f - radius && vel.y > 0.0f)
  {
    pos.y = 1.0f - radius;
    vel.y = -vel.y;
  }
}
