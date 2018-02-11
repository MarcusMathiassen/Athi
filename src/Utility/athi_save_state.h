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

#include "../athi_settings.h" // console
#include "../athi_utility.h" // file_exists

//#include <algorithm>
#include <vector>   // vector
#include <fstream>  // ifstream, ofstream

template <class T>
static void write_member(std::ostream& FILE,  T member)
{
    // Store its size
    size_t size = sizeof(T);
    FILE.write(reinterpret_cast<char*>(&size), sizeof(size));
    // Store its contents
    FILE.write(reinterpret_cast<char*>(&member), sizeof(T));
}

template <class T>
static void read_member(std::istream& FILE,  T& member)
{
    // Store its size
    size_t size = sizeof(T);
    FILE.read(reinterpret_cast<char*>(&size), sizeof(size));
    // Store its contents
    FILE.read(reinterpret_cast<char*>(&member), sizeof(T));
}

static string get_size(size_t size)
{
    static const char *SIZES[] = { "B", "kB", "MB", "GB" };
    size_t div = 0;
    size_t rem = 0;

    while (size >= 1024 && div < (sizeof SIZES / sizeof *SIZES))
    {
        rem = (size % 1024);
        div++;
        size /= 1024;
    }

    return std::to_string((float)size + (float)rem / 1024.0) +  SIZES[div];
}

template <class A, class B, class C>
static void write_data(
    const string& path,
    const vector<A>& particles,
    const vector<B>& colors,
    const vector<C>& transforms)
{

    const auto start_time = glfwGetTime();

    // Open up the file in binary mode and overwrite any previous data
    std::ofstream FILE(path, std::ios::out | std::ofstream::binary | std::ios::trunc);

    // Total size in bytes of each vector
    size_t s1 = particles.size()    * sizeof(A);
    size_t s2 = colors.size()       * sizeof(B);
    size_t s3 = transforms.size()   * sizeof(C);

    // Store size of the outer vector
    FILE.write(reinterpret_cast<char*>(&s1), sizeof(s1));
    FILE.write(reinterpret_cast<char*>(&s2), sizeof(s2));
    FILE.write(reinterpret_cast<char*>(&s3), sizeof(s3));

    // Now write each vector one by one
    for (const auto& v : particles)
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

    // Now write each vector one by one
    for (const auto& v : colors)
    {
        write_member(FILE, v.x);
        write_member(FILE, v.y);
        write_member(FILE, v.z);
        write_member(FILE, v.w);
    }

    // Now write each vector one by one
    for (const auto& v : transforms)
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

    const auto time_spent =  (glfwGetTime() - start_time) * 1000.0;
    console->warn("[IO WRITE {}ms] {}", time_spent, get_size(s1/sizeof(A) + s2/sizeof(B) + s3/sizeof(C)));
}

template <class A, class B, class C>
static void read_data(
    const string& path,
    vector<A>& particles,
    vector<B>& colors,
    vector<C>& transforms)
{
    // If the file does not exist just return
    if (!file_exists(path)) return;

    const auto start_time = glfwGetTime();

    std::ifstream FILE(path, std::ios::in | std::ofstream::binary);

    // Stores the sizes to be read in
    size_t s1,s2,s3;

    // Store size of the outer vector
    FILE.read(reinterpret_cast<char*>(&s1), sizeof(s1));
    FILE.read(reinterpret_cast<char*>(&s2), sizeof(s2));
    FILE.read(reinterpret_cast<char*>(&s3), sizeof(s3));

    // Resize our vectors to the correct size
    particles.resize(s1 / sizeof(A));
    colors.resize(s2 / sizeof(B));
    transforms.resize(s3 / sizeof(C));

    // Now read each vector one by one
    for (auto& v : particles)
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

    // Now read each vector one by one
    for (auto& v : colors)
    {
        read_member(FILE, v.x);
        read_member(FILE, v.y);
        read_member(FILE, v.z);
        read_member(FILE, v.w);
    }

    // Now read each vector one by one
    for (auto& v : transforms)
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

    const auto time_spent =  (glfwGetTime() - start_time) * 1000.0;
    console->warn("[IO READ {}ms] {}", time_spent, get_size(s1/sizeof(A) + s2/sizeof(B) + s3/sizeof(C)));
}
