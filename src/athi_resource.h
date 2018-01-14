#pragma once

#include "athi_typedefs.h"
#include "athi_settings.h"
#include <algorithm>
#include <unordered_map>

class ResourceManager {
private:
  std::unordered_map<std::string, std::uint32_t> resources;
public:
  void add_resource(const std::string &file, std::uint32_t resource) noexcept {
    resources[file] = resource;
    console->info("resource loaded: {}({})", file, resource);
  }

  std::uint32_t get_resource(const std::string &file) const noexcept {
    return resources.at(file);
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
