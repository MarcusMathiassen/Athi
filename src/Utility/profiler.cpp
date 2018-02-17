#include "profiler.h"

std::mutex cpu_profiler_mutex;
std::mutex gpu_profiler_mutex;
vector<std::tuple<string, f64>> cpu_profiles;
vector<std::tuple<string, f64>> gpu_profiles;
