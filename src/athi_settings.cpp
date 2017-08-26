#include "athi_settings.h"
#include "athi_typedefs.h"

f32 mouse_size{0.05f};
bool mouse_busy_UI{false};
bool mouse_grab_multiple{true};
bool show_mouse_grab_lines{false};

s32 screen_width;
s32 screen_height;

bool show_settings{false};
bool physics_gravity{false};
bool circle_collision{true};

f32 circle_size{0.03f};
vec4 circle_color{1.0f, 1.0f, 1.0f, 1.0f};

u64 comparisons{0};

bool draw_debug{false};
bool show_fps_info{true};

bool quadtree_show_only_occupied{true};
bool quadtree_active{false};
int quadtree_depth{5};
int quadtree_capacity{50};

bool voxelgrid_active{false};
int voxelgrid_parts{4};

f64 frametime;
f64 smoothed_frametime;
u32 framerate;
u32 framerate_limit{0};

f64 timestep;
f64 physics_frametime;
f64 smoothed_physics_frametime;
u32 physics_framerate;
u32 physics_FPS_limit{0};

bool vsync{true};

u32 variable_thread_count;
bool use_multithreading{false};

u32 cpu_cores;
u32 cpu_threads;
string cpu_brand;

bool openCL_active{false};

bool app_is_running{true};
bool settings_changed{false};

u32 universal_color_picker{0};
