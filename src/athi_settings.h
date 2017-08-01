#pragma once

#include "athi_typedefs.h"

extern bool show_settings;
extern bool physics_gravity;
extern bool circle_collision;

extern f32  circle_size;
extern vec4 circle_color;

extern bool quadtree_active;
extern u32 quadtree_depth;
extern u32 quadtree_capacity;

extern u32 framerate;
extern s32 framerate_limit;

extern f64 frametime;
extern f64 smoothed_frametime;

extern bool vsync;

extern u32    cpu_cores;
extern u32    cpu_threads;
extern string cpu_brand;

extern bool app_is_running;
extern bool settings_changed;
