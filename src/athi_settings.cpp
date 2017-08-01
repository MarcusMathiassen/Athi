#include "athi_settings.h"

bool show_settings{true};
bool physics_gravity{false};
bool circle_collision{true};

f32 circle_size{0.01f};
vec4 circle_color{1.0f, 1.0f, 1.0f, 1.0f};

bool quadtree_active{false};
u32 quadtree_depth{5};
u32 quadtree_capacity{25};

u32 framerate;
s32 framerate_limit{60};

f64 frametime;
f64 smoothed_frametime;

bool vsync{1};

u32    cpu_cores;
u32    cpu_threads;
string cpu_brand;

bool app_is_running{true};
bool settings_changed{false};
