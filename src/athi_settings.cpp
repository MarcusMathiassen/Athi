#include "athi_settings.h"
#include "athi_typedefs.h"

f64 frame_budget{1000.0 / 60.0};

f32 mouse_size{10.0f};
bool mouse_busy_UI{false};
bool mouse_grab_multiple{true};
bool show_mouse_grab_lines{false};
bool show_mouse_collision_box{false};
bool mouse_grab{true};
s32 screen_width;
s32 screen_height;
f32 px_scale{1.0f};

bool show_settings{true};
bool physics_gravity{false};
f32 gravity_force{9.81f};

float collision_energy_loss{0.99f};
bool circle_collision{true};
bool border_collision{true};

s32 physics_samples{8};

f32 circle_size{5.0f};
glm::vec4 circle_color{1.0f, 1.0f, 1.0f, 1.0f};

std::atomic<u64> comparisons{0};
std::atomic<u64> resolutions{0};

bool draw_debug{true};
bool color_particles{true};
bool draw_nodes{true};

bool quadtree_show_only_occupied{true};
bool quadtree_active{true};
s32 quadtree_depth{5};
s32 quadtree_capacity{50};

bool voxelgrid_active{false};
s32 voxelgrid_parts{16};

u16 monitor_refreshrate{60};

f64 frametime;
f64 smoothed_frametime;
s32 framerate;
s32 framerate_limit{0};

f64 render_frametime;
f64 smoothed_render_frametime;
s32 render_framerate;
s32 render_framerate_limit{0};

f64 timestep;
f64 physics_frametime;
f64 smoothed_physics_frametime;
s32 physics_framerate;
s32 physics_FPS_limit{0};

f32 time_scale{1.0f};
bool vsync{true};

s32 variable_thread_count;
bool use_multithreading{true};
bool use_libdispatch{false};

s32 cpu_cores;
s32 cpu_threads;
std::string cpu_brand;

bool openCL_active{false};

bool app_is_running{true};
bool settings_changed{false};

std::atomic<s32> universal_color_picker{0};
glm::vec4 background_color_dark = glm::vec4(0.15686f, 0.17255f, 0.20392f, 1.0f);
glm::vec4 background_color_light = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
glm::vec4 sw_color{pastel_red};
glm::vec4 se_color{pastel_gray};
glm::vec4 nw_color{pastel_orange};
glm::vec4 ne_color{pastel_pink};
