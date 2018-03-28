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

#include "../athi_utility.h"    // get_time
#include "athi_constant_globals.h" // DEBUG_MODE

#include "fixed_size_types.h" // u32, s32, etc.
#include <mutex> // std::mutex
#include <vector> // std::vector
#include <string> // std::string
using std::string;

extern std::mutex cpu_profiler_mutex;
extern std::mutex gpu_profiler_mutex;
extern std::vector<std::tuple<string, f64>> cpu_profiles;
extern std::vector<std::tuple<string, f64>> gpu_profiles;

class gpu_profile
{
private:
  f64     m_start_time{0.0};
  string  m_id;
  std::tuple<string, f64> *ptr_to_val;
public:
  gpu_profile(const char* id) noexcept : m_id(id)
  {
    if constexpr (DEBUG_MODE) {
      m_start_time = get_time();
      std::unique_lock<std::mutex> lck(gpu_profiler_mutex);
      ptr_to_val = &gpu_profiles.emplace_back(std::tuple<string,f64>(m_id, 0.0));
    }
  }
  ~gpu_profile() noexcept
  {
    if constexpr (DEBUG_MODE) {
      std::unique_lock<std::mutex> lck(gpu_profiler_mutex);
      *ptr_to_val = std::tuple<string,f64>(m_id, ((get_time() - m_start_time) * 1000.0));
    }
  }

  static void clear_profiles() noexcept
  {
    if constexpr (DEBUG_MODE) {
      std::unique_lock<std::mutex> lck(gpu_profiler_mutex);
      gpu_profiles.clear();
    }
  }
};
class cpu_profile
{
private:
  f64     m_start_time{0.0};
  string  m_id;
  std::tuple<string, f64> *ptr_to_val;
public:
  cpu_profile(const char* id) noexcept : m_id(id)
  {
    if constexpr (DEBUG_MODE) {
      m_start_time = get_time();
      std::unique_lock<std::mutex> lck(cpu_profiler_mutex);
      ptr_to_val = &cpu_profiles.emplace_back(std::tuple<string,f64>(m_id, 0.0));
    }
  }
  ~cpu_profile() noexcept
  {
    if constexpr (DEBUG_MODE) {
      std::unique_lock<std::mutex> lck(cpu_profiler_mutex);
      *ptr_to_val = std::tuple<string,f64>(m_id, ((get_time() - m_start_time) * 1000.0));
    }
  }

  static void clear_profiles() noexcept
  {
    if constexpr (DEBUG_MODE) {
      std::unique_lock<std::mutex> lck(cpu_profiler_mutex);
      cpu_profiles.clear();
    }
  }
};
