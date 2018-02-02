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

enum class OS { Apple, Windows, Linux };
#ifdef __APPLE__
  static constexpr OS os{OS::Apple};
#elif _WIN32
  static constexpr OS os{OS::Windows};
#else
  static constexpr OS os{OS::Linux};
#endif

enum class ThreadPoolSolution { AppleGCD, Dispatch, None };
extern ThreadPoolSolution threadpool_solution;

enum class TreeType { Quadtree, UniformGrid, None };
extern TreeType tree_type;

#ifdef NDEBUG
  static constexpr bool ONLY_RUNS_IN_DEBUG_MODE{true};
#else
  static constexpr bool ONLY_RUNS_IN_DEBUG_MODE{false};
#endif


// Engine
static constexpr bool multithreaded_engine{false};  // Rendering and Update are run on separate threads.
static constexpr bool use_textured_particles{true}; // Particles are rendered using textured billboards

// Constants
static constexpr f64 kPI = 3.14159265359;
static constexpr f64 kGravitationalConstant = 6.67408e-11;

// Colors
static const vec4 pastel_red(1, 0.411, 0.380, 1.0);
static const vec4 pastel_green(0.466, 0.745, 0.466, 1.0);
static const vec4 pastel_blue(0.466, 0.619, 0.796, 1.0);
static const vec4 pastel_yellow(0.992, 0.992, 0.588, 1.0);
static const vec4 pastel_purple(0.77647, 0.47059, 0.86667, 1.0);
static const vec4 pastel_pink(1, 0.819, 0.862, 1.0);
static const vec4 pastel_orange(1, 0.701, 0.278, 1.0);
static const vec4 pastel_gray(0.333, 0.333, 0.333, 1.0);