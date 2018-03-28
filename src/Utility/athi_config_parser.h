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

#include "fixed_size_types.h" // u64, s32, etc.
#include "console.h" // console

#include "../athi_utility.h"  // file_exists, etc
#include "../athi_settings.h"

#include <cctype> // toupper
#include <fstream> // std::ifstream
#include <type_traits> // is_integral, is_float
#include <unordered_map> // std::unordered_map

#include <vector> // std::vector
#include <string> // std::string
#include <glm/vec2.hpp> // glm::vec2
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4

using std::string;
using std::vector;
using glm::vec2;
using glm::vec3;
using glm::vec4;

// @Hack: the apple compiler doesnt have variant yet.
#ifdef __APPLE__
    #include <boost/variant.hpp>
    using boost::variant;
    using boost::get;
#else
    #include <variant> // variant, get_variable
    using std::variant;
    using std::get;
#endif

// Function forward decl
void refresh_variables() noexcept;
bool starts_with(const string& str, const string& s) noexcept;

string get_value_as_string(const string& var, const string& val) noexcept;
std::tuple<string, string> get_variable(const string& line) noexcept;
auto get_vec2(const string& str) noexcept;
auto get_vec3(const string& str) noexcept;
auto get_vec4(const string& str) noexcept;
auto get_bool(const string& str) noexcept;
auto get_float(const string& str) noexcept;
auto get_int(const string& str) noexcept;

bool is_float(const string& str) noexcept;
bool is_int(const string& str) noexcept;
bool is_bool(const string& str) noexcept;
bool is_string(const string& str) noexcept;
bool is_vec2(const string& str) noexcept;
bool is_vec3(const string& str) noexcept;
bool is_vec4(const string& str) noexcept;

string stringify_vec2(const vec2&v) noexcept;
string stringify_vec3(const vec3&v) noexcept;
string stringify_vec4(const vec4&v) noexcept;

void reload_variables() noexcept;
void save_variables() noexcept;
void init_variables() noexcept;
void refresh_variables() noexcept;

extern std::unordered_map<string, variant<string, float, vec2, vec3, vec4>> variable_map;

template <class T>
static void set_variable(T* var, const string& str)
{
    if constexpr (std::is_integral<T>::value)
    {
        if (variable_map.count(str)) {
            *var = get<float>(variable_map.at(str));
        }
    } else {
        if (variable_map.count(str)) {
            *var = get<T>(variable_map.at(str));
        }
    }
}
