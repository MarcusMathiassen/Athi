#pragma once

#include "../athi_typedefs.h"
#include "../athi_utility.h"
#include "../athi_settings.h"

static void init_variables();
static string eat_spaces(const string& str);
static bool starts_with(const string& str, const string& s);
static std::tuple<string, string> get_variable(const string& line);
static vector<string> split_string(const string& str, char delim);
static auto get_type(const string& val);

#include <cctype> // toupper
#include <unordered_map> // unordered_map
#include <boost/variant.hpp> // boost::variant, boost::get

std::unordered_map<string, boost::variant<string, bool, int, float, vec2, vec3, vec4>> variable_map;

static bool starts_with(const string& str, const string& s)
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

static auto get_vec2(const string& str)
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
static auto get_vec3(const string& str)
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
static auto get_vec4(const string& str)
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

static std::tuple<string, string> get_variable(const string& line)
{
    const auto result = split_string(line, ':');
    string variable = result[0];
    string value    = result[1];

    return {variable, value};
}

static string eat_spaces(const string& str)
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

static vector<string> split_string(const string& str, char delim)
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

static auto get_bool(const string& str)
{
    return (
        (str == "no"||str == "NO"||
         str == "false"||str == "FALSE") ? false :
        (str == "yes"||str == "YES" ||
        str == "true"||str == "TRUE") ? true : false);
}
static auto get_float(const string& str)
{
    return std::stof(str);
}

static auto get_int(const string& str)
{
    return std::stoi(str);
}

static bool is_float(const string& str)
{   
    for (auto c: str) if (c == '.') return true;
    return false;
}

static bool is_int(const string& str)
{   
    for (auto c: str) if (std::isdigit(c) && c != '.') return true;
    return false;
}

static bool is_bool(const string& str)
{   
    return (
        str == "no"||str == "NO"||
        str == "yes"||str == "YES"||
        str == "false"||str == "FALSE"||
        str == "true"||str == "TRUE")
    ? true : false;
}

static bool is_string(const string& str)
{   
    for (auto c: str) if (c == '"') return true;
    return false;
}

static bool is_vec2(const string& str)
{   
    return starts_with(str, "vec2");
}

static bool is_vec3(const string& str)
{   
    return starts_with(str, "vec3");
}

static bool is_vec4(const string& str)
{   
    return starts_with(str, "vec4");
}

static void init_variables()
{
    const auto file_data = get_content_of_file("../bin/config.ini");
    const auto no_spaces_data = eat_spaces(file_data);
    const auto lines = split_string(no_spaces_data, '\n');

    for (auto line: lines)
    {
        if (line.empty()) continue;
        if (line[0] == '\n') continue; // newline
        if (line[0] == '#') continue; // comment
        
        auto [var, val] = get_variable(line);
        console->info("{} = {}", var, val);

        if      (is_vec2(val))    { variable_map[var] = get_vec2(val); }
        else if (is_vec3(val))    { variable_map[var] = get_vec3(val); } 
        else if (is_vec4(val))    { variable_map[var] = get_vec4(val); }
        else if (is_string(val))  { variable_map[var] = val; }
        else if (is_float(val))   { variable_map[var] = get_float(val); }
        else if (is_int(val))     { variable_map[var] = get_int(val); }
        else if (is_bool(val))    { variable_map[var] = get_bool(val); }
    }

    if (variable_map.count("show_settings"))            show_settings           = boost::get<bool>(variable_map.at("show_settings"));
    if (variable_map.count("gravity"))                  physics_gravity          = boost::get<bool>(variable_map.at("gravity"));
    if (variable_map.count("gravity_force"))            gravity_force           = boost::get<float>(variable_map.at("gravity_force"));
    if (variable_map.count("time_scale"))               time_scale              = boost::get<float>(variable_map.at("time_scale"));
    if (variable_map.count("post_processing"))          post_processing         = boost::get<bool>(variable_map.at("post_processing"));
    if (variable_map.count("vsync"))                    vsync                   = boost::get<bool>(variable_map.at("vsync"));
    if (variable_map.count("background_color"))         background_color_dark   = boost::get<vec4>(variable_map.at("background_color"));
}
