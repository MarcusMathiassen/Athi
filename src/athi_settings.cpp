#include "athi_settings.h"

bool show_settings{true};
bool physics_gravity{false};
bool circle_collision{true};

f32 circle_size{0.03f};
vec4 circle_color{1.0f, 1.0f, 1.0f, 1.0f};

bool quadtree_active{false};
u32 quadtree_depth{5};
u32 quadtree_capacity{50};

bool voxelgrid_active{false};
u32 voxelgrid_parts{4};

f64  timestep;
f64 physics_frametime;
f64 smoothed_physics_frametime;
u32 physics_framerate;
u32 physics_updates_per_sec{0};

f64 frametime;
f64 smoothed_frametime;
u32 framerate;
u32 framerate_limit{60};

bool vsync{1};

u32 variable_thread_count;
bool use_multithreading{true};

u32    cpu_cores;
u32    cpu_threads;
string cpu_brand;

bool app_is_running{true};
bool settings_changed{false};
