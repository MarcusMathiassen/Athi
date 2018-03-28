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

#include "Utility/console.h" // console

#include <algorithm>            // find
#include <unordered_map>        // unordered_map

struct ResourceManager
{
  std::unordered_map<string, u32> resources;

  void add_resource(const string &file, u32 resource) noexcept {
    resources[file] = resource;
    console->info("resource loaded: {}", file);
  }

  void update_resource(const string &file, u32 resource) noexcept {
    resources[file] = resource;
    console->info("resource updated: {}", file);
  }

  u32 get_resource(const string &file) const noexcept {
    if (auto res = resources.find(file); res != resources.end()) {
      console->warn("resource already loaded: {}", file);
      return res->second;
    }
    return false;
  }
  bool is_loaded(const string &file) const noexcept {
    if (resources.empty()) return false;
    if (auto res = resources.find(file); res != resources.end()) {
      console->warn("resource already loaded: {}", file);
      return true;
    }
    return false;
  }
};
extern ResourceManager resource_manager;
