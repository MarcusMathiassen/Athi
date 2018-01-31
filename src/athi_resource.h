#pragma once

#include "athi_typedefs.h"

#include "athi_settings.h" // console
#include <algorithm> // find
#include <unordered_map> // unordered_map

class ResourceManager {
private:
  std::unordered_map<std::string, u32> resources;
public:
  void add_resource(const std::string &file, u32 resource) noexcept {
    resources[file] = resource;
    console->info("resource loaded: {}", file);
  }

  u32 get_resource(const std::string &file) const noexcept {
    if (auto res = resources.find(file); res != resources.end()) {
      console->warn("resource already loaded: {}", file);
      return res->second;
    }
    return false;
  }
  bool is_loaded(const std::string &file) const noexcept {
    if (resources.empty()) return false;
    if (auto res = resources.find(file); res != resources.end()) {
      console->warn("resource already loaded: {}", file);
      return false;
    }
    return true;
  }
};
extern ResourceManager resource_manager;
