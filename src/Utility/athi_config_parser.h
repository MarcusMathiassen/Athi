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

#include "../athi_typedefs.h"
#include "../athi_utility.h"
#include "../athi_utility.h" // file_exists
#include "../athi_settings.h"

static constexpr const char* path("../bin/config.ini");


static void init_variables() noexcept;
static string eat_spaces(const string& str) noexcept;
static bool starts_with(const string& str, const string& s) noexcept;
static std::tuple<string, string> get_variable(const string& line) noexcept;
static vector<string> split_string(const string& str, char delim) noexcept;
static auto get_type(const string& val) noexcept;
static void refresh_variables() noexcept;

#include <fstream> // ifstream
#include <type_traits> // is_integral, is_float
#include <cctype> // toupper
#include <unordered_map> // unordered_map
#include <boost/variant.hpp> // boost::variant, boost::get

static std::unordered_map<string, boost::variant<string, float, vec2, vec3, vec4>> variable_map;
static u64 last_write_time;

#ifdef _WIN32
#include <sys/stat.h>
#else
// Not Windows? Assume unix-like.
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

static u64 GetFileTimestamp(const char* filename) noexcept {
  u64 timestamp = 0;

#ifdef _WIN32
  struct __stat64 stFileInfo;
  if (_stat64(filename, &stFileInfo) == 0) {
    timestamp = stFileInfo.st_mtime;
  }
#else
  struct stat fileStat;

  if (stat(filename, &fileStat) == -1) {
    perror(filename);
    return 0;
  }

#ifdef __APPLE__
  timestamp = fileStat.st_mtimespec.tv_sec;
#else
  timestamp = fileStat.st_mtime;
#endif
#endif

  return timestamp;
}

static bool starts_with(const string& str, const string& s) noexcept
{
    u32 indx = 0;

    string type;
    string has_val;
    for (u32 i = 0; i < str.length(); ++i)
    {
        auto c = str[i];

        if (c == '(' || c == ')')
        {
            while (str[i+1] != ')')
            {
                auto k = str[++i];
                has_val += k;
            }
            break;
        }
        type += c;
    }

    if (s == type) return true;
    return false;
}

static auto get_vec2(const string& str) noexcept
{
    string has_val;
    for (u32 i = 0; i < str.length(); ++i)
    {
        auto c = str[i];

        if (c == '(' || c == ')')
        {
            while (str[i+1] != ')')
            {
                auto k = str[++i];
                has_val += k;
            }
            break;
        }
    }

    auto split = split_string(has_val, ',');
    vec2 temp;
    for (u32 i = 0; i < split.size(); ++i)
    {
        temp[i] = stof(split[i]);
    }
    return temp;
}
static auto get_vec3(const string& str) noexcept
{
    string has_val;
    for (u32 i = 0; i < str.length(); ++i)
    {
        auto c = str[i];

        if (c == '(' || c == ')')
        {
            while (str[i+1] != ')')
            {
                auto k = str[++i];
                has_val += k;
            }
            break;
        }
    }

    auto split = split_string(has_val, ',');
    vec3 temp;
    for (u32 i = 0; i < split.size(); ++i)
    {
        temp[i] = stof(split[i]);
    }
    return temp;

}
static auto get_vec4(const string& str) noexcept
{
    string has_val;
    for (u32 i = 0; i < str.length(); ++i)
    {
        auto c = str[i];

        if (c == '(' || c == ')')
        {
            while (str[i+1] != ')')
            {
                auto k = str[++i];
                has_val += k;
            }
            break;
        }
    }

    auto split = split_string(has_val, ',');
    vec4 temp;
    for (u32 i = 0; i < split.size(); ++i)
    {
        temp[i] = stof(split[i]);
    }
    return temp;
}

static std::tuple<string, string> get_variable(const string& line) noexcept
{
    const auto result = split_string(line, ':');
    const auto variable = result[0];
    const auto value    = result[1].substr(0, result[1].find('#'));

    return {variable, value};
}

static string eat_spaces(const string& str) noexcept
{
    string new_str;
    for (u32 i = 0; i < str.size(); ++i)
    {
        auto c = str[i];
        if (c == ' ' || c == '\t') continue;
        new_str += c;
    }
    return new_str;
}

static vector<string> split_string(const string& str, char delim) noexcept
{
    vector<string> strings;

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(delim, prev)) != std::string::npos)
    {
        strings.emplace_back(str.substr(prev, pos - prev));
        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.emplace_back(str.substr(prev));

    return strings;
}

static auto get_bool(const string& str) noexcept
{
    return 
        (str == "no"||str == "NO"||
        str == "off"||str == "OFF"||
         str == "false"||str == "FALSE") ? false :
        (str == "yes"||str == "YES" ||
        (str == "on"||str == "ON" ||
        str == "true"||str == "TRUE") ? true : false);
}

static auto get_float(const string& str) noexcept { return std::stof(str); }
static auto get_int(const string& str) noexcept  { return std::stoi(str); }

static bool is_float(const string& str) noexcept
{   
    bool res = true;
    for (auto c: str) 
        if (c != '.' && !std::isdigit(c))
            res = false;
    return res;
}

static bool is_int(const string& str) noexcept
{   
    for (auto c: str) if (std::isdigit(c)) return true;
    return false;
}

static bool is_bool(const string& str) noexcept
{   
    return (    str == "no"||str == "NO"||
                str == "off"||str == "OFF"||
                str == "yes"||str == "YES"||
                str == "on"||str == "ON"||
                str == "false"||str == "FALSE"||
                str == "true"||str == "TRUE")
                ? true : false;
}

static bool is_string(const string& str) noexcept
{   
    int quote_count = 0;

    for (auto c: str) 
        if (c == '"') ++quote_count;

    if (quote_count == 2) return true;
    else return false;
}

static string remove_quotes(const string& str) noexcept
{   
    int quote_count = 0;
    string new_str;
    for (auto c: str) 
        if (c != '"') {
            new_str += c;
        }

    return new_str;
}

static string add_quotes(const string& str) noexcept
{   
    return '"' + str + '"';
}


static bool string_has(const string& str, char delim)
{
    return (str.find(delim) == std::string::npos) ? false : true;
}

static bool is_vec2(const string& str) noexcept { return starts_with(str, "vec2"); }
static bool is_vec3(const string& str) noexcept { return starts_with(str, "vec3"); }
static bool is_vec4(const string& str) noexcept { return starts_with(str, "vec4"); }

static void reload_variables() noexcept
{
    const auto timestamp = GetFileTimestamp(path);
    if (timestamp > last_write_time) 
    {
        last_write_time = timestamp;

        init_variables();
    }
}

static string stringify_vec2(const vec2&v) noexcept { return {"vec2(" + std::to_string(v.x) +", "+ std::to_string(v.y) + ")"}; }
static string stringify_vec3(const vec3&v) noexcept { return {"vec3(" + std::to_string(v.x) +", "+ std::to_string(v.y) +", "+ std::to_string(v.z) + ")"}; }
static string stringify_vec4(const vec4&v) noexcept { return {"vec4(" + std::to_string(v.x) +", "+ std::to_string(v.y) +", "+ std::to_string(v.z) +", "+ std::to_string(v.w) + ")"}; }

static string get_value_as_string(const string& var, const string& val) noexcept
{
     // If has a ',' its a list of things
    if (string_has(val, ','))
    {
        if      (is_vec2(val))   return stringify_vec2(boost::get<vec2>(variable_map.at(var)));        
        else if (is_vec3(val))   return stringify_vec3(boost::get<vec3>(variable_map.at(var))); 
        else if (is_vec4(val))   return stringify_vec4(boost::get<vec4>(variable_map.at(var))); 
    } else {
        if      (is_string(val)) return add_quotes(boost::get<string>(variable_map.at(var)));
        else if (is_float(val))  return std::to_string(boost::get<float>(variable_map.at(var)));
        else if (is_bool(val))   return (boost::get<float>(variable_map.at(var))) == 0 ? "NO" : "YES";
    }

    return string();
}

static void save_variables() noexcept
{
    refresh_variables();

    last_write_time = GetFileTimestamp(path);

    const auto file_data = get_content_of_file(path);
    const auto lines = split_string(file_data, '\n');

    string new_file_data;

    for (auto line: lines)
    {
        if (line.empty()) continue;

        const auto no_spaces_line = eat_spaces(line);

        if (no_spaces_line[0] == '\n' ||
            no_spaces_line[0] == '#')
        {
            continue;
        }

        const auto [var, val] = get_variable(no_spaces_line);

        const auto new_val = get_value_as_string(var, val);

        const auto pos = line.find(':');
        const auto temp = line.substr(0, pos);

        auto new_line = temp;
        new_line += ": " + new_val;

        new_file_data += new_line + "\n";
    }
    new_file_data += "\n";
    std::fstream file(path);
    file << new_file_data;
    console->warn("Config saved");
}

static void refresh_variables() noexcept
{
    variable_map["particle_texture"] = particle_texture;
    variable_map["acceleration_color_max"] = acceleration_color_max;
    variable_map["acceleration_color_min"] = acceleration_color_min;
    variable_map["air_resistance"] = air_resistance;
    variable_map["background_color"] = background_color;
    variable_map["blur_strength"] = blur_strength;
    variable_map["circle_color"] = circle_color;
    variable_map["circle_size"] = circle_size;
    variable_map["color_by_velocity_threshold"] = color_by_velocity_threshold;
    variable_map["color_particles"] = color_particles;
    variable_map["draw_circles"] = draw_circles;
    variable_map["draw_debug"] = draw_debug;
    variable_map["draw_lines"] = draw_lines;
    variable_map["draw_particles"] = draw_particles;
    variable_map["draw_rects"] = draw_rects;
    variable_map["gButtonHeight"] = gButtonHeight;
    variable_map["gButtonWidth"] = gButtonWidth;
    variable_map["gravity"] = gravity;
    variable_map["has_random_velocity"] = has_random_velocity;
    variable_map["is_particles_colored_by_acc"] = is_particles_colored_by_acc;
    variable_map["monitor_refreshrate"] = monitor_refreshrate;
    variable_map["mouse_busy_UI"] = mouse_busy_UI;
    variable_map["mouse_size"] = mouse_size;
    variable_map["multithreaded_particle_update"] = multithreaded_particle_update;
    variable_map["num_vertices_per_particle"] = num_vertices_per_particle;
    variable_map["openCL_active"] = openCL_active;
    variable_map["physics_samples"] = physics_samples;
    variable_map["post_processing"] = post_processing;
    variable_map["post_processing_samples"] = post_processing_samples;
    variable_map["px_scale"] = px_scale;
    variable_map["quadtree_active"] = quadtree_active;
    variable_map["quadtree_capacity"] = quadtree_capacity;
    variable_map["quadtree_depth"] = quadtree_depth;
    variable_map["quadtree_show_only_occupied"] = quadtree_show_only_occupied;
    variable_map["random_velocity_force"] = random_velocity_force;
    variable_map["show_mouse_collision_box"] = show_mouse_collision_box;
    variable_map["show_mouse_grab_lines"] = show_mouse_grab_lines;
    variable_map["show_settings"] = show_settings;
    variable_map["time_scale"] = time_scale;
    variable_map["tree_optimized_size"] = tree_optimized_size;
    variable_map["uniformgrid_parts"] = uniformgrid_parts;
    variable_map["use_gravitational_force"] = use_gravitational_force;
    variable_map["use_libdispatch"] = use_libdispatch;
    variable_map["use_multithreading"] = use_multithreading;
    variable_map["use_uniformgrid"] = use_uniformgrid;
    variable_map["variable_thread_count"] = variable_thread_count;
    variable_map["vsync"] = vsync;
    variable_map["wireframe_mode"] = wireframe_mode;
    variable_map["circle_collision"] = circle_collision;
    variable_map["border_collision"] = border_collision;
    variable_map["draw_debug"] = draw_debug;
    variable_map["window_pos"] = window_pos;
    variable_map["screen_width"] = screen_width;
    variable_map["screen_height"] = screen_height;
    variable_map["framebuffer_width"] = framebuffer_width;
    variable_map["framebuffer_height"] = framebuffer_height;
}

static const string default_config = 
    "particle_texture                        : \"particle_texture.png\"\n"
    "acceleration_color_max                  : vec4(0.315000, 1.000000, 0.000000, 0.100000)\n"
    "acceleration_color_min                  : vec4(0.891325, 0.122900, 0.122900, 1.000000)\n"
    "air_resistance                          : 0.990000\n"
    "background_color                        : vec4(0.129000, 0.143000, 0.225000, 1.000000)\n"
    "blur_strength                           : 2.000000\n"
    "circle_color                            : vec4(0.623282, 0.914004, 0.160079, 1.000000)\n"
    "circle_size                             : 5.000000\n"
    "color_by_velocity_threshold             : 0.003000\n"
    "color_particles                         : NO\n"
    "draw_circles                            : YES\n"
    "draw_debug                              : NO\n"
    "draw_lines                              : YES\n"
    "draw_particles                          : YES\n"
    "draw_rects                              : YES\n"
    "gButtonHeight                           : 25.000000\n"
    "gButtonWidth                            : 200.000000\n"
    "gravity                                 : 0.000000\n"
    "has_random_velocity                     : YES\n"
    "is_particles_colored_by_acc             : NO\n"
    "monitor_refreshrate                     : 60.000000\n"
    "mouse_busy_UI                           : NO\n"
    "mouse_size                              : 40.635612\n"
    "multithreaded_particle_update           : YES\n"
    "num_vertices_per_particle               : 36.000000\n"
    "openCL_active                           : NO\n"
    "physics_samples                         : 8.000000\n"
    "post_processing                         : YES\n"
    "post_processing_samples                 : 4.000000\n"
    "px_scale                                : 2.000000\n"
    "quadtree_active                         : YES\n"
    "quadtree_capacity                       : 100.000000\n"
    "quadtree_depth                          : 10.000000\n"
    "quadtree_show_only_occupied             : NO\n"
    "random_velocity_force                   : 5.000000\n"
    "show_mouse_collision_box                : YES\n"
    "show_mouse_grab_lines                   : YES\n"
    "show_settings                           : YES\n"
    "time_scale                              : 1.000000\n"
    "tree_optimized_size                     : YES\n"
    "uniformgrid_parts                       : 16.000000\n"
    "use_gravitational_force                 : NO\n"
    "use_libdispatch                         : YES\n"
    "use_multithreading                      : YES\n"
    "use_uniformgrid                         : NO\n"
    "variable_thread_count                   : 8.000000\n"
    "vsync                                   : 1.000000\n"
    "wireframe_mode                          : NO\n"
    "circle_collision                        : NO\n"
    "border_collision                        : NO\n"
    "draw_debug                              : NO\n"
    "window_pos                              : vec2(256, 256)\n"
    "screen_width                            : 512\n"
    "screen_height                           : 512\n"
    "framebuffer_width                       : 512\n"
    "framebuffer_height                      : 512\n";

template <class T>
static void set_variable(T* var, const string& str)
{
    if constexpr (std::is_integral<T>::value)
    {
        if (variable_map.count(str)) {
            *var = boost::get<float>(variable_map.at(str));
        }
    } else {
        if (variable_map.count(str)) {
            *var = boost::get<T>(variable_map.at(str));
        }
    }
}

static void init_variables() noexcept
{

    if (!file_exists(path)) {
        std::ofstream file(path);
        file << default_config;
    }

    const auto file_data = get_content_of_file(path);
    last_write_time = GetFileTimestamp(path);
    const auto no_spaces_data = eat_spaces(file_data);
    const auto lines = split_string(no_spaces_data, '\n');

    for (auto line: lines)
    {
        if (line.empty()) continue;
        if (line[0] == '\n') continue; // newline
        if (line[0] == '#') continue; // comment
        
        const auto [var, val] = get_variable(line);

        if (val.empty()) { continue; }

        // If has a ',' its a list of things
        if (string_has(val, ','))
        {
            if      (is_vec2(val))    { variable_map[var] = get_vec2(val); }
            else if (is_vec3(val))    { variable_map[var] = get_vec3(val); } 
            else if (is_vec4(val))    { variable_map[var] = get_vec4(val); }   
        } else {
            if      (is_string(val))  { variable_map[var] = remove_quotes(val); }
            else if (is_float(val))   { variable_map[var] = get_float(val); }
            else if (is_bool(val))    { variable_map[var] = get_bool(val); }
        }
    }

    set_variable(&particle_texture, "particle_texture");
    set_variable(&acceleration_color_max, "acceleration_color_max");
    set_variable(&acceleration_color_min, "acceleration_color_min");
    set_variable(&air_resistance, "air_resistance");
    set_variable(&background_color, "background_color");
    set_variable(&blur_strength, "blur_strength");
    set_variable(&circle_color, "circle_color");
    set_variable(&circle_size, "circle_size");
    set_variable(&color_by_velocity_threshold, "color_by_velocity_threshold");
    set_variable(&color_particles, "color_particles");
    set_variable(&draw_circles, "draw_circles");
    set_variable(&draw_debug, "draw_debug");
    set_variable(&draw_lines, "draw_lines");
    set_variable(&draw_particles, "draw_particles");
    set_variable(&draw_rects, "draw_rects");
    set_variable(&gButtonHeight, "gButtonHeight");
    set_variable(&gButtonWidth, "gButtonWidth");
    set_variable(&gravity, "gravity");
    set_variable(&has_random_velocity, "has_random_velocity");
    set_variable(&is_particles_colored_by_acc, "is_particles_colored_by_acc");
    set_variable(&monitor_refreshrate, "monitor_refreshrate");
    set_variable(&mouse_busy_UI, "mouse_busy_UI");
    set_variable(&mouse_size, "mouse_size");
    set_variable(&multithreaded_particle_update, "multithreaded_particle_update");
    set_variable(&num_vertices_per_particle, "num_vertices_per_particle");
    set_variable(&openCL_active, "openCL_active");
    set_variable(&physics_samples, "physics_samples");
    set_variable(&post_processing, "post_processing");
    set_variable(&post_processing_samples, "post_processing_samples");
    set_variable(&px_scale, "px_scale");
    set_variable(&quadtree_active, "quadtree_active");
    set_variable(&quadtree_capacity, "quadtree_capacity");
    set_variable(&quadtree_depth, "quadtree_depth");
    set_variable(&quadtree_show_only_occupied, "quadtree_show_only_occupied");
    set_variable(&random_velocity_force, "random_velocity_force");
    set_variable(&show_mouse_collision_box, "show_mouse_collision_box");
    set_variable(&show_mouse_grab_lines, "show_mouse_grab_lines");
    set_variable(&show_settings, "show_settings");
    set_variable(&time_scale, "time_scale");
    set_variable(&tree_optimized_size, "tree_optimized_size");
    set_variable(&uniformgrid_parts, "uniformgrid_parts");
    set_variable(&use_gravitational_force, "use_gravitational_force");
    set_variable(&use_libdispatch, "use_libdispatch");
    set_variable(&use_multithreading, "use_multithreading");
    set_variable(&use_uniformgrid, "use_uniformgrid");
    set_variable(&variable_thread_count, "variable_thread_count");
    set_variable(&vsync, "vsync");
    set_variable(&wireframe_mode, "wireframe_mode");
    set_variable(&circle_collision, "circle_collision");
    set_variable(&border_collision, "border_collision");
    set_variable(&draw_debug, "draw_debug");
    set_variable(&window_pos, "window_pos");
    set_variable(&screen_width, "screen_width");
    set_variable(&screen_height, "screen_height");    
    set_variable(&framebuffer_width, "framebuffer_width");
    set_variable(&framebuffer_height, "framebuffer_height");

    console->warn("Config loaded");
}
