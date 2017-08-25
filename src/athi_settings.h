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
extern int  voxelgrid_parts;

extern bool quadtree_show_only_occupied;
extern bool quadtree_active;
extern int quadtree_depth;
extern int quadtree_capacity;

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

static vec4 pastel_red     (238.0f / 255.0f, 0.0f, 0.0f, 1.0f);
static vec4 pastel_gray    (85 / 255.0f, 85 / 255.0f, 85 / 255.0f, 1.0f);
static vec4 pastel_green   (33.0f / 255.0f,176.0f/ 255.0f,1.0/ 255.0f, 1.0f);
static vec4 pastel_orange  (255.0f/ 255.0f,165.0f/ 255.0f,0.0f, 1.0f);
static vec4 pastel_yellow  (255.0f/ 255.0f,242.0f/ 255.0f,63.0f/ 255.0f, 1.0f);
static vec4 pastel_purple  (100 / 255.0f, 20 / 255.0f, 100 / 255.0f, 1.0f);
static vec4 pastel_blue    (63.0f/ 255.0f,150.0f/ 255.0f,255.0f/ 255.0f, 1.0f);
static vec4 pastel_pink    (255.0f/ 255.0f, 20.0f/ 255.0f,147.0/ 255.0f, 1.0f);