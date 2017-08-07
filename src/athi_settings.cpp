#include "athi_settings.h"
#include "athi_typedefs.h"

bool show_settings{true};
bool physics_gravity{false};
bool circle_collision{true};

f32 circle_size{0.03f};
vec4 circle_color{1.0f, 1.0f, 1.0f, 1.0f};

bool draw_debug{false};

bool quadtree_active{false};
u32 quadtree_depth{6};
u32 quadtree_capacity{25};

bool voxelgrid_active{false};
u32 voxelgrid_parts{4};

f64 frametime;
f64 smoothed_frametime;
u32 framerate;
u32 framerate_limit{60};

f64 timestep;
f64 physics_frametime;
f64 smoothed_physics_frametime;
u32 physics_framerate;
u32 physics_updates_per_sec{0};

bool vsync{1};

u32 variable_thread_count;
bool use_multithreading{true};

u32 cpu_cores;
u32 cpu_threads;
string cpu_brand;

bool app_is_running{true};
bool settings_changed{false};

u32 universal_color_picker{0};

vec4 pastel_red(255 / 255.0f, 105 / 255.0f, 97 / 255.0f, 1.0f);
vec4 pastel_gray(85 / 255.0f, 85 / 255.0f, 85 / 255.0f, 1.0f);
vec4 pastel_green(119 / 255.0f, 190 / 255.0f, 119 / 255.0f, 1.0f);
vec4 pastel_orange(255 / 255.0f, 179 / 255.0f, 71 / 255.0f, 1.0f);
vec4 pastel_yellow(253 / 255.0f, 253 / 255.0f, 150 / 255.0f, 1.0f);
vec4 pastel_purple(100 / 255.0f, 20 / 255.0f, 100 / 255.0f, 1.0f);
vec4 pastel_blue(119 / 255.0f, 158 / 255.0f, 203 / 255.0f, 1.0f);
vec4 pastel_pink(255 / 255.0f, 209 / 255.0f, 220 / 255.0f, 1.0f);
