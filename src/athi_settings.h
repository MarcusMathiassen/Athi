// Copyright (c) 2018 Marcus Mathiassen

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.


#pragma once

#include "athi_typedefs.h"

#include "./Utility/athi_constant_globals.h"
#include "./Renderer/athi_framebuffer.h"
#include "imgui.h"
#include <atomic>

#include "../dep/Universal/spdlog/spdlog.h" // Console logging
extern std::shared_ptr<spdlog::logger> console;

extern string particle_texture;

enum class TreeType { Quadtree, UniformGrid, None };
extern TreeType tree_type;

enum class MouseOption { Color, GravityWell, Drag, Delete, None };
extern MouseOption mouse_option;

extern s32 mouse_radio_options;
extern s32 tree_radio_option;

extern float gButtonWidth;
extern float gButtonHeight;

extern bool wireframe_mode;
extern bool draw_particles;

extern bool cycle_particle_color;
extern s32 num_vertices_per_particle;
extern bool is_particles_colored_by_acc;
extern bool has_random_velocity;
extern f32 random_velocity_force;
extern f32 color_by_velocity_threshold;

extern vector<FrameBuffer> framebuffers;
extern bool post_processing;
extern f64 frame_budget;

extern f32 mouse_size;
extern bool show_mouse_collision_box;
extern bool mouse_busy_UI;
extern bool show_mouse_grab_lines;
extern bool mouse_grab;

extern s32 screen_width;
extern s32 screen_height;
extern s32 framebuffer_width;
extern s32 framebuffer_height;
extern vec2 window_pos;

extern f32 px_scale;

extern bool show_settings;
extern bool use_gravitational_force;
extern f32 gravity;
extern f32 gravitational_constant;
extern f32 air_resistance;
extern float collision_energy_loss;
extern bool circle_collision;
extern bool border_collision;

extern bool multithreaded_particle_update;
extern s32 physics_samples;
extern s32 post_processing_samples;
extern s32 blur_strength;

extern f32 circle_size;
extern vec4 circle_color;
extern std::atomic<u64> comparisons;
extern std::atomic<u64> resolutions;

extern bool draw_debug;

extern bool draw_rects;
extern bool draw_circles;
extern bool draw_lines;

extern bool color_particles;
extern bool draw_nodes;
extern bool show_fps_info;

extern bool use_uniformgrid;
extern s32 uniformgrid_parts;

extern bool tree_optimized_size;
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

extern vec4 acceleration_color_min;
extern vec4 acceleration_color_max;

extern vec4 background_color;

extern vec4 sw_color;
extern vec4 se_color;
extern vec4 nw_color;
extern vec4 ne_color;