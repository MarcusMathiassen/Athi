#include "athi_settings.h"
#include "athi_typedefs.h"

float mouse_size{5.0f};
bool mouse_busy_UI{false};
bool mouse_grab_multiple{true};
bool show_mouse_grab_lines{true};
bool mouse_grab{true};
int32_t screen_width;
int32_t screen_height;
float px_scale{1.0f};

bool show_settings{false};
bool physics_gravity{false};
float gravity_force{9.81f};
bool circle_collision{true};

int32_t physics_samples{1};

float circle_size{3.0f};
glm::vec4 circle_color{1.0f, 1.0f, 1.0f, 1.0f};

std::atomic<uint64_t> comparisons{0};
std::atomic<uint64_t> resolutions{0};

bool draw_debug{false};
bool color_particles{true};
bool draw_nodes{true};
bool show_fps_info{true};

bool quadtree_show_only_occupied{true};
bool quadtree_active{false};
int32_t quadtree_depth{5};
int32_t quadtree_capacity{50};

bool voxelgrid_active{false};
int32_t voxelgrid_parts{16};

double frametime;
double smoothed_frametime;
int32_t framerate;
int32_t framerate_limit{0};

double render_frametime;
double smoothed_render_frametime;
int32_t render_framerate;
int32_t render_framerate_limit{0};

double timestep;
double physics_frametime;
double smoothed_physics_frametime;
int32_t physics_framerate;
int32_t physics_FPS_limit{0};

bool slowmotion{false};
bool vsync{true};

int32_t variable_thread_count;
bool use_multithreading{false};

int32_t cpu_cores;
int32_t cpu_threads;
std::string cpu_brand;

bool openCL_active{false};

bool app_is_running{true};
bool settings_changed{false};

std::atomic<int32_t> universal_color_picker{0};
