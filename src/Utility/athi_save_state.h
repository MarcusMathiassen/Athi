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

#include <glm/vec2.hpp>

#include "../athi_settings.h" // console
#include "../athi_utility.h" // file_exists

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>
#include <fstream>

static const char* path_particle("../bin/particles.dat");
static const char* path_color("../bin/colors.dat");
static const char* path_transform("../bin/transforms.dat");

template <class T> 
static void write_member(std::ostream& FILE,  T member)
{
    size_t size = sizeof(T);
    // Store its size
    FILE.write(reinterpret_cast<char*>(&size), sizeof(size));
    // Store its contents
    FILE.write(reinterpret_cast<char*>(&member), sizeof(T));
}

template <class T> 
static void read_member(std::istream& FILE,  T& member)
{
    size_t size = sizeof(T);

    // Store its size
    FILE.read(reinterpret_cast<char*>(&size), sizeof(size));
    // Store its contents
    FILE.read(reinterpret_cast<char*>(&member), sizeof(T));
}

template <class T>
static void write_particle_data(const vector<T>& data)
{
    std::ofstream FILE(path_particle, std::ios::out | std::ofstream::binary | std::ios::trunc);

    size_t s1 = data.size() * sizeof(T);

    // Store size of the outer vector
    FILE.write(reinterpret_cast<char*>(&s1), sizeof(s1));
    
    // Now write each vector one by one
    for (const auto& v : data)
    {
        write_member(FILE, v.id);
        write_member(FILE, v.pos.x);
        write_member(FILE, v.pos.y);
        write_member(FILE, v.vel.x);
        write_member(FILE, v.vel.y);
        write_member(FILE, v.acc.x);
        write_member(FILE, v.acc.y);
        write_member(FILE, v.mass);
        write_member(FILE, v.radius);
    }
    FILE.close();   

    console->warn("[IO WRITE] Particle data: {} bytes", (s1));
}

template <class T>
static void read_particle_data(vector<T>& data)
{
    if (!file_exists(path_particle)) return;
    std::ifstream FILE(path_particle, std::ios::in | std::ofstream::binary);

    size_t s1;

    // Store size of the outer vector
    FILE.read(reinterpret_cast<char*>(&s1), sizeof(s1));
    
    data.resize(s1 / sizeof(T));

    // Now write each vector one by one
    for (auto &v: data)
    {
        read_member(FILE, v.id);
        read_member(FILE, v.pos.x);
        read_member(FILE, v.pos.y);
        read_member(FILE, v.vel.x);
        read_member(FILE, v.vel.y);
        read_member(FILE, v.acc.x);
        read_member(FILE, v.acc.y);
        read_member(FILE, v.mass);
        read_member(FILE, v.radius);
    }

    FILE.close();   
    console->warn("[IO READ] Particle data: {} bytes", (s1));
}

template <class T>
static void write_color_data(const vector<T>& data)
{
    std::ofstream FILE(path_color, std::ios::out | std::ofstream::binary | std::ios::trunc);

    size_t s1 = data.size() * sizeof(T);

    // Store size of the outer vector
    FILE.write(reinterpret_cast<char*>(&s1), sizeof(s1));
    
    // Now write each vector one by one
    for (const auto& v : data)
    {
        write_member(FILE, v.x);
        write_member(FILE, v.y);
        write_member(FILE, v.z);
        write_member(FILE, v.w);
    }
    FILE.close();   
    console->warn("[IO WRITE] Color data: {} bytes", (s1));
}

template <class T>
static void read_color_data(vector<T>& data)
{
    if (!file_exists(path_color)) return;
    std::ifstream FILE(path_color, std::ios::in | std::ofstream::binary);

    size_t s1;

    // Store size of the outer vector
    FILE.read(reinterpret_cast<char*>(&s1), sizeof(s1));
    
    data.resize(s1 / sizeof(T));

    // Now write each vector one by one
    for (auto &v: data)
    {
        read_member(FILE, v.x);
        read_member(FILE, v.y);
        read_member(FILE, v.z);
        read_member(FILE, v.w);
    }

    FILE.close();   
    console->warn("[IO READ] Color data: {} bytes", (s1));
}

template <class T>
static void write_transform_data(const vector<T>& data)
{
    std::ofstream FILE(path_transform, std::ios::out | std::ofstream::binary | std::ios::trunc);

    size_t s1 = data.size() * sizeof(T);

    // Store size of the outer vector
    FILE.write(reinterpret_cast<char*>(&s1), sizeof(s1));
    
    // Now write each vector one by one
    for (const auto& v : data)
    {
        write_member(FILE, v.pos.x);
        write_member(FILE, v.pos.y);
        write_member(FILE, v.pos.z);

        write_member(FILE, v.rot.x);
        write_member(FILE, v.rot.y);
        write_member(FILE, v.rot.z);

        write_member(FILE, v.scale.x);
        write_member(FILE, v.scale.y);
        write_member(FILE, v.scale.z);
    }
    FILE.close();   
    console->warn("[IO WRITE] Transform data: {} bytes", (s1));
}

template <class T>
static void read_transform_data(vector<T>& data)
{
    if (!file_exists(path_transform)) return;

    std::ifstream FILE(path_transform, std::ios::in | std::ofstream::binary);

    size_t s1;

    // Store size of the outer vector
    FILE.read(reinterpret_cast<char*>(&s1), sizeof(s1));
    
    data.resize(s1 / sizeof(T));

    // Now write each vector one by one
    for (auto &v: data)
    {
        read_member(FILE, v.pos.x);
        read_member(FILE, v.pos.y);
        read_member(FILE, v.pos.z);

        read_member(FILE, v.rot.x);
        read_member(FILE, v.rot.y);
        read_member(FILE, v.rot.z);

        read_member(FILE, v.scale.x);
        read_member(FILE, v.scale.y);
        read_member(FILE, v.scale.z);
    }

    FILE.close();   
    console->warn("[IO READ] Transform data: {} bytes", (s1));
}
