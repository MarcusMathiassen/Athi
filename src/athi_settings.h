#pragma once

#include "athi_typedefs.h"

extern f32 mouse_size;
extern bool mouse_grab_multiple;
extern bool mouse_busy_UI;

extern s32 screen_width;
extern s32 screen_height;

extern bool show_settings;
extern bool physics_gravity;
extern bool circle_collision;

extern f32  circle_size;
extern vec4 circle_color;
extern u64 comparisons;

extern bool draw_debug;
extern bool show_fps_info;

extern bool voxelgrid_active;
extern u32 voxelgrid_parts;

extern bool quadtree_show_only_occupied;
extern bool quadtree_active;
extern u32 quadtree_depth;
extern u32 quadtree_capacity;

extern f64  timestep;
extern f64  physics_frametime;
extern f64  smoothed_physics_frametime;
extern u32  physics_framerate;
extern u32  physics_FPS_limit;

extern u32 framerate;
extern u32 framerate_limit;

extern f64 deltatime;
extern f64 frametime;
extern f64 smoothed_frametime;

extern bool vsync;

extern u32 variable_thread_count;
extern bool use_multithreading;

extern u32    cpu_cores;
extern u32    cpu_threads;
extern string cpu_brand;

extern bool openCL_active;

extern bool app_is_running;
extern bool settings_changed;

extern u32 universal_color_picker;

extern vec4 pastel_red;
extern vec4 pastel_gray;
extern vec4 pastel_green;
extern vec4 pastel_orange;
extern vec4 pastel_yellow;
extern vec4 pastel_purple;
extern vec4 pastel_blue;
extern vec4 pastel_pink;
