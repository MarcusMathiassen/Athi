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
size_t quadtree_depth{5};
size_t quadtree_capacity{50};

bool voxelgrid_active{false};
size_t voxelgrid_parts{4};

double frametime;
double smoothed_frametime;
unsigned int framerate;
unsigned int framerate_limit{0};

double timestep;
double physics_frametime;
double smoothed_physics_frametime;
unsigned int physics_framerate;
unsigned int physics_FPS_limit{0};

bool slowmotion{false};
bool vsync{true};

unsigned int variable_thread_count;
bool use_multithreading{false};

unsigned int cpu_cores;
unsigned int cpu_threads;
string cpu_brand;

bool openCL_active{false};

bool app_is_running{true};
bool settings_changed{false};

unsigned int universal_color_picker{0};
