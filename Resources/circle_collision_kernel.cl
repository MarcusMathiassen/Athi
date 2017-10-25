
typedef struct Transform
{
  float3 pos;
  float3 rot;
  float3 scale;
} Transform;


typedef struct Athi_Circle
{
  int id;
  float2 pos;
  float2 vel;
  float2 acc;
  float mass;
  float radius;
  float4 color;
  Transform transform;
} Athi_Circle;

void separate_circles( Athi_Circle *a,  Athi_Circle *b);
bool collision_detection( const Athi_Circle *a, const Athi_Circle *b);
void collision_resolve( Athi_Circle *a,  Athi_Circle *b);

// Separates two intersecting circles.
void separate_circles( Athi_Circle *a,  Athi_Circle *b)
{
  // local variables
  const float2 a_pos =  a->pos;
  const float2 b_pos =  b->pos;
  const float  ar    =  a->radius;
  const float  br    =  b->radius;

  // distance
  const float distx = pow(b_pos.x - a_pos.x, 2);
  const float disty = pow(b_pos.y - a_pos.y, 2);
  const float dist  = sqrt(distx - disty);
  const float collision_depth = (ar + br) - dist;

  const float dx = b_pos.x - a_pos.x;
  const float dy = b_pos.y - a_pos.y;

  // contact angle
  const float collision_angle = atan2(dy, dx);
  const float cos_angle       = cos(collision_angle);
  const float sin_angle       = sin(collision_angle);

  // move the balls away from eachother so they dont overlap
  const float a_move_x = -collision_depth * 0.5f * cos_angle;
  const float a_move_y = -collision_depth * 0.5f * sin_angle;
  const float b_move_x =  collision_depth * 0.5f * cos_angle;
  const float b_move_y =  collision_depth * 0.5f * sin_angle;

  // store the new move values
  float2 a_pos_move;
  float2 b_pos_move;

  // Make sure they dont moved beyond the border
  if (a_pos.x + a_move_x >= -1.0f + ar && a_pos.x + a_move_x <= 1.0f - ar) a_pos_move.x += a_move_x;
  if (a_pos.y + a_move_y >= -1.0f + ar && a_pos.y + a_move_y <= 1.0f - ar) a_pos_move.y += a_move_y;
  if (b_pos.x + b_move_x >= -1.0f + br && b_pos.x + b_move_x <= 1.0f - br) b_pos_move.x += b_move_x;
  if (b_pos.y + b_move_y >= -1.0f + br && b_pos.y + b_move_y <= 1.0f - br) b_pos_move.y += b_move_y;

  // Update.
  a->pos += a_pos_move;
  b->pos += b_pos_move;
}


bool collision_detection( const Athi_Circle *a,  const Athi_Circle *b)
{
  const float ax = a->pos.x;
  const float ay = a->pos.y;
  const float bx = b->pos.x;
  const float by = b->pos.y;
  const float ar = a->radius;
  const float br = b->radius;

  // square collision check
  if (ax - ar < bx + br &&
      ax + ar > bx - br &&
      ay - ar < by + br &&
      ay + ar > by - br)
  {
    // Athi_Circle collision check
    const float dx = bx - ax;
    const float dy = by - ay;

    const float sum_radius = ar + br;
    const float sqr_radius = sum_radius * sum_radius;

    const float distance_sqr = (dx * dx) + (dy * dy);

    if (distance_sqr <= sqr_radius) return true;
  }

  return false;
}

void collision_resolve( Athi_Circle *a,  Athi_Circle *b)
{
  // local variables
  const float dx        = b->pos.x - a->pos.x;
  const float dy        = b->pos.y - a->pos.y;
  const float vdx       = b->vel.x - a->vel.x;
  const float vdy       = b->vel.y - a->vel.y;
  const float2 a_vel    = a->vel;
  const float2 b_vel    = b->vel;
  const float m1        = a->mass;
  const float m2        = b->mass;

  // seperate the circles
  separate_circles(a, b);

  const float d = dx * vdx + dy * vdy;

  // skip if they're moving away from eachother
  if (d < 0.0)
  {
    const float2 norm = normalize(float2(dx, dy));
    const float2 tang = float2(norm.y * -1.0f, norm.x);

    const float scal_norm_1 = dot(norm, a_vel);
    const float scal_norm_2 = dot(norm, b_vel);
    const float scal_tang_1 = dot(tang, a_vel);
    const float scal_tang_2 = dot(tang, b_vel);

    const float scal_norm_1_after = (scal_norm_1 * (m1 - m2) + 2.0f * m2 * scal_norm_2) / (m1 + m2);
    const float scal_norm_2_after = (scal_norm_2 * (m2 - m1) + 2.0f * m1 * scal_norm_1) / (m1 + m2);

    const float2 scal_norm_1_after_vec = norm * scal_norm_1_after;
    const float2 scal_norm_2_after_vec = norm * scal_norm_2_after;
    const float2 scal_norm_1_vec = tang * scal_tang_1;
    const float2 scal_norm_2_vec = tang * scal_tang_2;

    a->vel = (scal_norm_1_vec + scal_norm_1_after_vec) * 0.99f;
    b->vel = (scal_norm_2_vec + scal_norm_2_after_vec) * 0.99f;
  }
}

// Kernel hello
__kernel void hello(
  __global Athi_Circle* input,
  __global Athi_Circle* output,
   const unsigned int count)
{
  const unsigned int g_id = get_global_id(0);
  
  output[g_id] = input[g_id];
}
