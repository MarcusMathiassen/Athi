#pragma once

#include "athi_typedefs.h"

extern f32 mouse_size;
extern bool mouse_grab_multiple;
extern bool mouse_busy_UI;
extern bool show_mouse_grab_lines;

extern s32 screen_width;
extern s32 screen_height;

extern bool show_settings;
extern bool physics_gravity;
extern bool circle_collision;

extern f32 circle_size;
extern vec4 circle_color;
extern u64 comparisons;

extern bool draw_debug;
extern bool show_fps_info;

extern bool voxelgrid_active;
extern size_t voxelgrid_parts;

extern bool quadtree_show_only_occupied;
extern bool quadtree_active;
extern size_t quadtree_depth;
extern size_t quadtree_capacity;

extern f64 timestep;
extern f64 physics_frametime;
extern f64 smoothed_physics_frametime;
extern u32 physics_framerate;
extern u32 physics_FPS_limit;

extern u32 framerate;
extern u32 framerate_limit;

extern f64 deltatime;
extern f64 frametime;
extern f64 smoothed_frametime;

extern bool slowmotion;
extern bool vsync;

extern u32 variable_thread_count;
extern bool use_multithreading;

extern u32 cpu_cores;
extern u32 cpu_threads;
extern string cpu_brand;

extern bool openCL_active;

extern bool app_is_running;
extern bool settings_changed;

extern u32 universal_color_picker;

static const glm::vec4 pastel_red(1, 0.411, 0.380, 1.0);
static const glm::vec4 pastel_green(0.466, 0.745, 0.466, 1.0);
static const glm::vec4 pastel_blue(0.466, 0.619, 0.796, 1.0);
static const glm::vec4 pastel_yellow(0.992, 0.992, 0.588, 1.0);
static const glm::vec4 pastel_purple(0.77647, 0.47059, 0.86667, 1.0);
static const glm::vec4 pastel_pink(1, 0.819, 0.862, 1.0);
static const glm::vec4 pastel_orange(1, 0.701, 0.278, 1.0);
static const glm::vec4 pastel_gray(0.333, 0.333, 0.333, 1.0);
