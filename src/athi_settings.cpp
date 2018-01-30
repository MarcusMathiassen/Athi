#include "athi_settings.h"
#include "athi_typedefs.h"
#include "athi_framebuffer.h"

MouseOption mouse_option{MouseOption::Drag};
TreeType tree_type{TreeType::Quadtree};
ThreadPoolSolution threadpool_solution{ThreadPoolSolution::Dispatch};

std::shared_ptr<spdlog::logger> console;

std::vector<FrameBuffer> framebuffers;
f64 frame_budget{1000.0 / 60.0};

s32 mouse_radio_options = static_cast<s32>(MouseOption::Drag);
s32 tree_radio_option = 0;

u32 num_vertices_per_particle = 36;
bool is_particles_colored_by_acc = true;
bool has_random_velocity = true;

bool post_processing{true};
f32 mouse_size{10.0f};
bool mouse_busy_UI{false};
bool show_mouse_grab_lines{false};
bool show_mouse_collision_box{false};
bool mouse_grab{true};
s32 screen_width;
s32 screen_height;
f32 px_scale{1.0f};

bool show_settings{true};
bool physics_gravity{false};
bool use_gravitational_force{false};
f32 gravity_force{9.81f};
f32 gravitational_constant{6.674e-11};
f32 air_drag{0.9};

f32 collision_energy_loss{0.99f};
bool circle_collision{true};
bool border_collision{true};

s32 physics_samples{8};
s32 post_processing_samples{2};
s32 blur_strength{2};

f32 circle_size{5.0f};
vec4 circle_color{1.0f, 1.0f, 1.0f, 1.0f};

std::atomic<u64> comparisons{0};
std::atomic<u64> resolutions{0};

bool draw_debug{false};
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
string cpu_brand;

bool openCL_active{false};

bool app_is_running{true};
bool settings_changed{false};

vec4 acceleration_color_min{vec4(1,1,1,1)};
vec4 acceleration_color_max{pastel_blue};

std::atomic<s32> universal_color_picker{0};
vec4 background_color_dark = vec4(18.0f/255.0f, 20.0f/255.0f, 25.0f/255.0f, 1.0f);
vec4 background_color_light = vec4(0.7f, 0.7f, 0.7f, 1.0f);
vec4 sw_color{pastel_red};
vec4 se_color{pastel_gray};
vec4 nw_color{pastel_orange};
vec4 ne_color{pastel_pink};
