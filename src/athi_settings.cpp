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


#include "athi_settings.h"

std::vector<FrameBuffer> framebuffers;
f64 frame_budget{1.0 / 60.0};

// --------------
// Renderer
// --------------

bool draw_debug                 = false;
bool color_particles            = false;
bool draw_nodes                 = true;

bool draw_rects                 = true;
bool draw_circles               = true;
bool draw_lines                 = true;

s32 post_processing_samples{2};
s32 blur_strength{2};

vec4 acceleration_color_min{vec4(1,1,1,1)};
vec4 acceleration_color_max{vec4(0.315f, 1.000f, 0.000f, 0.100f)};

vec4 default_gui_text_color{black};
vec4 background_color = vec4(1, 1, 1, 1);

// --------------
// Particle System
// --------------

string particle_texture{"particle_texture"};

s32 num_vertices_per_particle = 36;
bool is_particles_colored_by_acc = false;
bool has_random_velocity = true;
f32 random_velocity_force = 5.0f;
f32 color_by_velocity_threshold = 0.003f;

float gButtonWidth{200.f};
float gButtonHeight{25.f};

bool cycle_particle_color{true};
bool wireframe_mode{false};
bool draw_particles{true};
bool post_processing{true};

f32 mouse_size{10.0f};
bool mouse_busy_UI{false};
bool show_mouse_grab_lines{true};
bool show_mouse_collision_box{true};
bool mouse_grab{true};

f32 px_scale{1.0f};

bool show_settings{true};
bool use_gravitational_force{false};
f32 gravity{9.81f};
f32 gravitational_constant{6.674e-11f};
f32 air_resistance{0.9f};

f32 collision_energy_loss{0.99f};
bool circle_collision{true};
bool border_collision{true};

bool multithreaded_particle_update{true};
s32 physics_samples{8};

f32 circle_size{5.0f};
vec4 circle_color{1.0f, 1.0f, 1.0f, 1.0f};

bool tree_optimized_size{true};
bool quadtree_show_only_occupied{false};
bool quadtree_active{true};
s32 quadtree_depth{10};
s32 quadtree_capacity{100};

bool use_uniformgrid{false};
s32 uniformgrid_parts{256};

f32 time_scale{1.0f};
bool vsync{true};

bool use_multithreading{true};
bool use_libdispatch{false};

bool openCL_active{false};


// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Do not touch beyond here
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

s32 screen_width{512};
s32 screen_height{512};

s32 framebuffer_width{512};
s32 framebuffer_height{512};

vec2 window_pos{512.0f/2.0f, 512.0f/2.0f};

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

u16 monitor_refreshrate{60};

bool app_is_running{true};
bool settings_changed{false};

s32 cpu_cores;
s32 cpu_threads;
string cpu_brand;
s32 variable_thread_count;
std::atomic<s32> universal_color_picker{0};

std::atomic<u64> comparisons{0};
std::atomic<u64> resolutions{0};

s32 mouse_radio_options = static_cast<s32>(MouseOption::Drag);
s32 tree_radio_option = 0;

MouseOption mouse_option{MouseOption::Drag};
TreeType tree_type{TreeType::Quadtree};
