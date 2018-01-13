#pragma once

#include <atomic>
#include "athi_typedefs.h"
#include "imgui.h"

extern f64 frame_budget;

extern f32 mouse_size;
extern bool mouse_grab_multiple;
extern bool show_mouse_collision_box;
extern bool mouse_busy_UI;
extern bool show_mouse_grab_lines;
extern bool mouse_grab;

extern s32 screen_width;
extern s32 screen_height;
extern f32 px_scale;

extern bool show_settings;
extern bool physics_gravity;
extern f32 gravity_force;

extern float collision_energy_loss;
extern bool circle_collision;
extern bool border_collision;

extern s32 physics_samples;

extern f32 circle_size;
extern glm::vec4 circle_color;
extern std::atomic<u64> comparisons;
extern std::atomic<u64> resolutions;

extern bool draw_debug;
extern bool color_particles;
extern bool draw_nodes;
extern bool show_fps_info;

extern bool voxelgrid_active;
extern s32 voxelgrid_parts;

extern bool quadtree_show_only_occupied;
extern bool quadtree_active;
extern s32 quadtree_depth;
extern s32 quadtree_capacity;

extern f64 timestep;
extern f64 physics_frametime;
extern f64 smoothed_physics_frametime;
extern s32 physics_framerate;
extern s32 physics_FPS_limit;

extern f64 render_frametime;
extern f64 smoothed_render_frametime;
extern s32 render_framerate;
extern s32 render_framerate_limit;

extern u16 monitor_refreshrate;

extern s32 framerate;
extern s32 framerate_limit;

extern f64 deltatime;
extern f64 frametime;
extern f64 smoothed_frametime;

extern f32 time_scale;
extern bool vsync;

extern s32 variable_thread_count;
extern bool use_multithreading;
extern bool use_libdispatch;
extern s32 cpu_cores;
extern s32 cpu_threads;
extern string cpu_brand;

extern bool openCL_active;

extern bool app_is_running;
extern bool settings_changed;

extern std::atomic<s32> universal_color_picker;

extern glm::vec4 sw_color;
extern glm::vec4 se_color;
extern glm::vec4 nw_color;
extern glm::vec4 ne_color;

extern glm::vec4 background_color_dark;
extern glm::vec4 background_color_light;
static const glm::vec4 pastel_red(1, 0.411, 0.380, 1.0);
static const glm::vec4 pastel_green(0.466, 0.745, 0.466, 1.0);
static const glm::vec4 pastel_blue(0.466, 0.619, 0.796, 1.0);
static const glm::vec4 pastel_yellow(0.992, 0.992, 0.588, 1.0);
static const glm::vec4 pastel_purple(0.77647, 0.47059, 0.86667, 1.0);
static const glm::vec4 pastel_pink(1, 0.819, 0.862, 1.0);
static const glm::vec4 pastel_orange(1, 0.701, 0.278, 1.0);
static const glm::vec4 pastel_gray(0.333, 0.333, 0.333, 1.0);
