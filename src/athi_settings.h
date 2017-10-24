#pragma once

#include "athi_typedefs.h"

extern float mouse_size;
extern bool mouse_grab_multiple;
extern bool mouse_busy_UI;
extern bool show_mouse_grab_lines;

extern int32_t screen_width;
extern int32_t screen_height;

extern bool show_settings;
extern bool physics_gravity;
extern bool circle_collision;

extern float circle_size;
extern glm::vec4 circle_color;
extern int32_t comparisons;

extern bool draw_debug;
extern bool show_fps_info;

extern bool voxelgrid_active;
extern int32_t voxelgrid_parts;

extern bool quadtree_show_only_occupied;
extern bool quadtree_active;
extern int32_t quadtree_depth;
extern int32_t quadtree_capacity;

extern double timestep;
extern double physics_frametime;
extern double smoothed_physics_frametime;
extern int32_t physics_framerate;
extern int32_t physics_FPS_limit;

extern double render_frametime;
extern double smoothed_render_frametime;
extern int32_t render_framerate;
extern int32_t render_framerate_limit;

extern int32_t framerate;
extern int32_t framerate_limit;

extern double deltatime;
extern double frametime;
extern double smoothed_frametime;

extern bool slowmotion;
extern bool vsync;

extern int32_t variable_thread_count;
extern bool use_multithreading;

extern int32_t cpu_cores;
extern int32_t cpu_threads;
extern string cpu_brand;

extern bool openCL_active;

extern bool app_is_running;
extern bool settings_changed;

extern int32_t universal_color_picker;

static const glm::vec4 pastel_red(1, 0.411, 0.380, 1.0);
static const glm::vec4 pastel_green(0.466, 0.745, 0.466, 1.0);
static const glm::vec4 pastel_blue(0.466, 0.619, 0.796, 1.0);
static const glm::vec4 pastel_yellow(0.992, 0.992, 0.588, 1.0);
static const glm::vec4 pastel_purple(0.77647, 0.47059, 0.86667, 1.0);
static const glm::vec4 pastel_pink(1, 0.819, 0.862, 1.0);
static const glm::vec4 pastel_orange(1, 0.701, 0.278, 1.0);
static const glm::vec4 pastel_gray(0.333, 0.333, 0.333, 1.0);
