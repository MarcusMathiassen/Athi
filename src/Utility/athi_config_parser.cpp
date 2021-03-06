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

#include "athi_config_parser.h"


static const string path = "../bin/config.ini";

std::unordered_map<string, variant<string, float, vec2, vec3, vec4>> variable_map;

static u64 last_write_time;

static const string default_config =
"# ---------------- Particle options ----------------\n"
"\n"
"particle_texture                        : \"particle_texture_9.png\"\n"
"\n"
"num_vertices_per_particle               : 36.000000\n"
"\n"
"circle_collision                        : YES\n"
"border_collision                        : YES\n"
"\n"
"cycle_particle_color                    : NO\n"
"\n"
"color_particles                         : NO\n"
"color_by_velocity_threshold             : 0.002000\n"
"\n"
"random_velocity_force                   : 10.000000\n"
"\n"
"circle_size                             : 45.060001\n"
"circle_color                            : vec4(0.069776, 0.924528, 0.198796, 1.000000)\n"
"\n"
"acceleration_color_max                  : vec4(1.000000, 0.000000, 0.000060, 1.000000)\n"
"acceleration_color_min                  : vec4(1.000000, 0.999990, 0.999990, 1.000000)\n"
"\n"
"\n"
"# ---------------- Simulation options ----------------\n"
"gravity                                 : 10.981000\n"
"time_scale                              : 1.000000\n"
"use_gravitational_force                 : NO\n"
"air_resistance                          : 0.990000\n"
"physics_samples                         : 1.000000\n"
"\n"
"# ------- Tree options -------\n"
"\n"
"tree_optimized_size                     : YES\n"
"\n"
"quadtree_active                         : YES\n"
"quadtree_capacity                       : 100.000000\n"
"quadtree_depth                          : 10.000000\n"
"\n"
"use_uniformgrid                         : NO\n"
"uniformgrid_parts                       : 4.000000\n"
"\n"
"\n"
"# ---------------- Render options ----------------\n"
"\n"
"background_color                        : vec4(0.000001, 0.000001, 0.000001, 1.000000)\n"
"text_color                              : vec4(0.000000, 0.000000, 0.000000, 1.000000)\n"
"vsync                                   : 1.000000\n"
"\n"
"post_processing                         : YES\n"
"post_processing_samples                 : 4.000000\n"
"blur_strength                           : 2.000000\n"
"\n"
"# ---------------- Engine options ----------------\n"
"\n"
"use_libdispatch                         : YES\n"
"use_multithreading                      : YES\n"
"openCL_active                           : NO\n"
"\n"
"show_settings                           : YES\n"
"variable_thread_count                   : 8.000000\n"
"\n"
"\n"
"# ---------------- Misc options ----------------\n"
"\n"
"gButtonHeight                           : 26.596001\n"
"gButtonWidth                            : 26.596001\n"
"has_random_velocity                     : YES\n"
"is_particles_colored_by_acc             : YES\n"
"monitor_refreshrate                     : 60.000000\n"
"mouse_busy_UI                           : NO\n"
"mouse_size                              : 82.159004\n"
"multithreaded_particle_update           : YES\n"
"\n"
"# ---------------- Debug options ----------------\n"
"\n"
"draw_debug                              : 0.000000\n"
"wireframe_mode                          : YES\n"
"quadtree_show_only_occupied             : YES\n"
"\n"
"show_mouse_grab_lines                   : YES\n"
"show_mouse_collision_box                : YES\n"
"\n"
"\n"
"draw_circles                            : YES\n"
"draw_lines                              : YES\n"
"draw_particles                          : YES\n"
"draw_rects                              : YES\n"
"\n"
"\n"
"\n"
"# ---------------- DONT TOUCH ANY OF THESE ----------------\n"
"\n"
"window_pos                              : vec2(1460.000000, 690.000000)\n"
"screen_width                            : 1658.000000\n"
"screen_height                           : 1076.000000\n"
"framebuffer_width                       : 1658.000000\n"
"framebuffer_height                      : 1076.000000\n"
"px_scale                                : 1.000000\n";

static const vector<string> available_vars = {
{"particle_texture"},
{"acceleration_color_max"},
{"acceleration_color_min"},
{"air_resistance"},
{"background_color"},
{"text_color"},
{"blur_strength"},
{"circle_color"},
{"circle_size"},
{"color_by_velocity_threshold"},
{"color_particles"},
{"draw_circles"},
{"draw_lines"},
{"draw_particles"},
{"draw_rects"},
{"gButtonHeight"},
{"gButtonWidth"},
{"gravity"},
{"has_random_velocity"},
{"is_particles_colored_by_acc"},
{"monitor_refreshrate"},
{"mouse_busy_UI"},
{"mouse_size"},
{"multithreaded_particle_update"},
{"num_vertices_per_particle"},
{"openCL_active"},
{"physics_samples"},
{"post_processing"},
{"post_processing_samples"},
{"px_scale"},
{"quadtree_active"},
{"quadtree_capacity"},
{"quadtree_depth"},
{"quadtree_show_only_occupied"},
{"random_velocity_force"},
{"show_mouse_collision_box"},
{"show_mouse_grab_lines"},
{"show_settings"},
{"time_scale"},
{"tree_optimized_size"},
{"uniformgrid_parts"},
{"use_gravitational_force"},
{"use_libdispatch"},
{"use_multithreading"},
{"use_uniformgrid"},
{"variable_thread_count"},
{"vsync"},
{"wireframe_mode"},
{"circle_collision"},
{"border_collision"},
{"draw_debug"},
{"window_pos"},
{"screen_width"},
{"screen_height"},
{"framebuffer_width"},
{"framebuffer_height"},
{"cycle_particle_color"},
};

bool starts_with(const string& str, const string& s) noexcept
{
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

auto get_vec2(const string& str) noexcept
{
    string has_val;
    for (u32 i = 0; i < str.length(); ++i) {
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
    vec2 temp{0.0f, 0.0f};
    for (u32 i = 0; i < split.size(); ++i)
    {
        temp[i] = stof(split[i]);
    }
    return temp;
}
auto get_vec3(const string& str) noexcept
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
    vec3 temp{0.0f, 0.0f, 0.0f};
    for (u32 i = 0; i < split.size(); ++i)
    {
        temp[i] = stof(split[i]);
    }
    return temp;

}
auto get_vec4(const string& str) noexcept
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
    vec4 temp{ 0.0f, 0.0f, 0.0f, 0.0f };
    for (u32 i = 0; i < split.size(); ++i)
    {
        temp[i] = stof(split[i]);
    }
    return temp;
}

std::tuple<string, string> get_variable(const string& line) noexcept
{
    const auto result = split_string(line, ':');

    if (result.size() == 1)
    {
        return {"",""};
    }

    const auto lhs = result[0];
    auto rhs = result[1];

    if (rhs.empty() || rhs == "\n") rhs = "";

    const auto variable = lhs;
    const auto value    = rhs.substr(0, rhs.find({' ', '\n'}));

    return {variable, value};
}

auto get_bool(const string& str) noexcept
{
    return
        (str == "no"||str == "NO"||
        str == "off"||str == "OFF"||
         str == "false"||str == "FALSE") ? false :
        (str == "yes"||str == "YES" ||
        (str == "on"||str == "ON" ||
        str == "true"||str == "TRUE") ? true : false);
}

auto get_float(const string& str) noexcept { return std::stof(str); }
auto get_int(const string& str) noexcept  { return std::stoi(str); }

bool is_float(const string& str) noexcept
{
    bool res = true;
    for (auto c: str)
        if (c != '.' && !std::isdigit(c))
            res = false;
    return res;
}

bool is_int(const string& str) noexcept
{
    for (auto c: str) if (std::isdigit(c)) return true;
    return false;
}

bool is_bool(const string& str) noexcept
{
    return (    str == "no"||str == "NO"||
                str == "off"||str == "OFF"||
                str == "yes"||str == "YES"||
                str == "on"||str == "ON"||
                str == "false"||str == "FALSE"||
                str == "true"||str == "TRUE")
                ? true : false;
}

bool is_string(const string& str) noexcept
{
    int quote_count = 0;

    for (auto c: str)
        if (c == '"') ++quote_count;

    if (quote_count == 2) return true;
    else return false;
}


string get_value_as_string(const string& var, const string& val) noexcept
{
     // If has a ',' its a list of things
    if (string_has(val, ','))
    {
        if      (is_vec2(val))   return stringify_vec2(get<vec2>(variable_map.at(var)));
        else if (is_vec3(val))   return stringify_vec3(get<vec3>(variable_map.at(var)));
        else if (is_vec4(val))   return stringify_vec4(get<vec4>(variable_map.at(var)));
    } else {
        if      (is_string(val)) return add_quotes(get<string>(variable_map.at(var)));
        else if (is_float(val))  return std::to_string(get<float>(variable_map.at(var)));
        else if (is_bool(val))   return (get<float>(variable_map.at(var))) == 0 ? "NO" : "YES";
    }

    return string();
}

bool is_vec2(const string& str) noexcept { return starts_with(str, "vec2"); }
bool is_vec3(const string& str) noexcept { return starts_with(str, "vec3"); }
bool is_vec4(const string& str) noexcept { return starts_with(str, "vec4"); }

string stringify_vec2(const vec2&v) noexcept { return {"vec2(" + std::to_string(v.x) +", "+ std::to_string(v.y) + ")"}; }
string stringify_vec3(const vec3&v) noexcept { return {"vec3(" + std::to_string(v.x) +", "+ std::to_string(v.y) +", "+ std::to_string(v.z) + ")"}; }
string stringify_vec4(const vec4&v) noexcept { return {"vec4(" + std::to_string(v.x) +", "+ std::to_string(v.y) +", "+ std::to_string(v.z) +", "+ std::to_string(v.w) + ")"}; }

void reload_variables() noexcept
{
    const auto timestamp = get_file_time_stamp(path);
    if (timestamp > last_write_time)
    {
        last_write_time = timestamp;

        init_variables();
    }
}

void save_variables() noexcept
{
    refresh_variables();

    last_write_time = get_file_time_stamp(path);

    const auto file_data = get_content_of_file(path);

    // Split the file into lines separated by newlines
    const auto lines = split_string(file_data, '\n');

    string new_file_data;

    // Read line by line.. but skip the last newline
    for (size_t i = 0; i < lines.size()-1; ++i)
    {
        auto& line = lines[i];

        // Get the line without spaces or tabs.
        const auto no_spaces_line = eat_chars(line, {' ', '\t'});

        // If the line is a newline or a comment
        //  just output the line and go to the next line.
        if (no_spaces_line[0] == '\n' || no_spaces_line[0] == '#')
        {
            new_file_data += line + '\n';
            continue;
        }

        // The variable and value do not have a newline at the end.
        const auto [var, val] = get_variable(no_spaces_line);

        // If the variable or value is empty. Break.
        if (var.empty() || val.empty())
        {
            new_file_data += '\n';
            continue;
        }

        // stringify the value..
        const auto new_val = get_value_as_string(var, val);

        const auto pos = line.find(':');
        const auto new_line = line.substr(0, pos);

        // and output to our file
        new_file_data += new_line + ": " + new_val + "\n";
    }

    std::ofstream file(path, std::ios::trunc);
    file << new_file_data;
    console->warn("Config saved");
}

bool is_var(const string& var) noexcept
{
    for (const auto &avail_var: available_vars)
    {
        if (var == avail_var) return true;
    }
    return false;
}

void init_variables() noexcept
{
    if (!file_exists(path)) {
        std::ofstream file(path);
        file << default_config;
    }

    last_write_time = get_file_time_stamp(path);

    const auto file_data = get_content_of_file(path);
    const auto no_spaces_data = eat_chars(file_data, {' ', '\t'});
    const auto lines = split_string(no_spaces_data, '\n');

    for (auto line: lines)
    {
        if (line.empty()) continue;
        if (line[0] == '\0') break; // eof
        if (line[0] == '\n') continue; // newline
        if (line[0] == '#') continue; // comment
        if (line[0] == ':') continue; // comment

        const auto [var, val] = get_variable(line);

        if (val.empty() || var.empty()) { continue; }

        if (!is_var(var)) continue;

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
    set_variable(&text_color, "text_color");
    set_variable(&blur_strength, "blur_strength");
    set_variable(&circle_color, "circle_color");
    set_variable(&circle_size, "circle_size");
    set_variable(&color_by_velocity_threshold, "color_by_velocity_threshold");
    set_variable(&color_particles, "color_particles");
    set_variable(&draw_circles, "draw_circles");
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
    set_variable(&cycle_particle_color, "cycle_particle_color");

    console->warn("Config loaded");
}

void refresh_variables() noexcept
{
    variable_map["particle_texture"] = particle_texture;
    variable_map["acceleration_color_max"] = acceleration_color_max;
    variable_map["acceleration_color_min"] = acceleration_color_min;
    variable_map["air_resistance"] = air_resistance;
    variable_map["background_color"] = background_color;
    variable_map["text_color"] = text_color;
    variable_map["blur_strength"] = blur_strength;
    variable_map["circle_color"] = circle_color;
    variable_map["circle_size"] = circle_size;
    variable_map["color_by_velocity_threshold"] = color_by_velocity_threshold;
    variable_map["color_particles"] = color_particles;
    variable_map["draw_circles"] = draw_circles;
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
    variable_map["cycle_particle_color"] = cycle_particle_color;
}
