#include "athi_settings.h"
#include "athi_framebuffer.h"
#include "athi_typedefs.h"

TreeType tree_type{TreeType::Quadtree};
ThreadPoolSolution threadpool_solution{ThreadPoolSolution::Dispatch};

std::shared_ptr<spdlog::logger> console;

std::vector<FrameBuffer> framebuffers;
bool post_processing{true};
double frame_budget{1000.0 / 60.0};

float mouse_size{10.0f};
bool mouse_busy_UI{false};
bool mouse_grab_multiple{true};
bool show_mouse_grab_lines{false};
bool show_mouse_collision_box{false};
bool mouse_grab{true};
std::int32_t screen_width;
std::int32_t screen_height;
float px_scale{1.0f};

bool show_settings{true};
bool physics_gravity{false};
bool use_gravitational_force{false};
float gravity_force{9.81f};
float gravitational_constant{6.674e-6};

float collision_energy_loss{0.99f};
bool circle_collision{true};
bool border_collision{false};

std::int32_t physics_samples{4};

float circle_size{5.0f};
glm::vec4 circle_color{1.0f, 1.0f, 1.0f, 1.0f};

std::atomic<u64> comparisons{0};
std::atomic<u64> resolutions{0};

bool draw_debug{false};
bool color_particles{true};
bool draw_nodes{true};

bool quadtree_show_only_occupied{true};
bool quadtree_active{true};
std::int32_t quadtree_depth{5};
std::int32_t quadtree_capacity{50};

bool voxelgrid_active{false};
std::int32_t voxelgrid_parts{16};

u16 monitor_refreshrate{60};

double frametime;
double smoothed_frametime;
std::int32_t framerate;
std::int32_t framerate_limit{0};

double render_frametime;
double smoothed_render_frametime;
std::int32_t render_framerate;
std::int32_t render_framerate_limit{0};

double timestep;
double physics_frametime;
double smoothed_physics_frametime;
std::int32_t physics_framerate;
std::int32_t physics_FPS_limit{0};

float time_scale{1.0f};
bool vsync{true};

std::int32_t variable_thread_count;
bool use_multithreading{true};
bool use_libdispatch{false};

std::int32_t cpu_cores;
std::int32_t cpu_threads;
std::string cpu_brand;

bool openCL_active{false};

bool app_is_running{true};
bool settings_changed{false};

std::atomic<std::int32_t> universal_color_picker{0};
glm::vec4 background_color_dark = glm::vec4(0.15686f, 0.17255f, 0.20392f, 1.0f);
glm::vec4 background_color_light = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
glm::vec4 sw_color{pastel_red};
glm::vec4 se_color{pastel_gray};
glm::vec4 nw_color{pastel_orange};
glm::vec4 ne_color{pastel_pink};
