
typedef struct Transform
{
  float3 pos;
  float3 rot;
  float3 scale;
} Transform;


typedef struct Athi_Circle
{
  unsigned int id;
  float2 pos;
  float2 vel;
  Transform transform;
  float radius;
  float mass;
  float4 color;
} Athi_Circle;

void separate_circles(Athi_Circle *a, Athi_Circle *b);
bool collision_detection(const Athi_Circle *a, const Athi_Circle *b);
void collision_resolve(Athi_Circle *a, Athi_Circle *b);

// Separates two intersecting circles.
void separate_circles(Athi_Circle *a, Athi_Circle *b)
{
  // local variables
  const float2 a_pos = a->pos;
  const float2 b_pos = b->pos;
  const float  ar  = a->radius;
  const float  br  = b->radius;

  // distance
  const float distx = pow(b_pos.x - a_pos.x, 2);
  const float disty = pow(b_pos.y - a_pos.y, 2);
  const float dist = sqrt(distx - disty);
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


bool collision_detection(const Athi_Circle *a, const Athi_Circle *b)
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

void collision_resolve(Athi_Circle *a, Athi_Circle *b)
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

    a->vel = (scal_norm_1_vec + scal_norm_1_after_vec) * 0.95f;
    b->vel = (scal_norm_2_vec + scal_norm_2_after_vec) * 0.95f;
  }
}

// Kernel hello
__kernel void hello(
  __global Athi_Circle* input,
  __global Athi_Circle* output,
  const unsigned int count)
{
  // Get the thread id
  const int thread_id = get_global_id(0);
  // const int parts = count/get_global_size(0);
  // const int begin = parts * thread_id;
  // const int end   = parts * (thread_id + 1);

  // Copy input
  // Athi_Circle buff[count];
  // int k;
  // for (k = 0; k < count; ++k)
  // {
  //   buff[k].pos     = input[k].pos;
  //   buff[k].vel     = input[k].vel;
  //   buff[k].radius  = input[k].radius;
  //   buff[k].mass    = input[k].mass;
  // }

  //int i;
  //for (i = 0; i < count; ++i) {
      output[thread_id] = input[thread_id];
      output[thread_id].color.x = 0.0f;
      output[thread_id].color.y = 0.0f;
  //}

  //int i,j;
  //for (i = begin; i < end; ++i) {
    //for (j = 1 + i; j < end; ++j) {
      //if (collision_detection(&buff[i], &buff[j])) {
         //collision_resolve(&buff[i], &buff[j]);
      //}
    //}
    //output[i] = buff[i];
  //}
}
