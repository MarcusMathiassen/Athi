#include "profiler.h"

#include <mutex>

Profiler *current{nullptr};

static std::mutex mutex;
static vector<Profiler> profile_buffer;

static void add_profile(Profiler* profiler) noexcept
{
    // Lock
    std::unique_lock<std::mutex> lck(mutex);

    // Add it to our buffer
    profile_buffer.emplace_back(*profiler);
}

static void set_current_profile(Profiler* profiler) noexcept
{
    // Lock
    std::unique_lock<std::mutex> lck(mutex);

    // If this is the first added
    if (current == nullptr)
        current = profiler;
    else
        current->child = profiler;
}

Profiler::Profiler(const char* name) : name(name)
{
    thread_id = std::this_thread::get_id();

    start_time = get_time();

    set_current_profile(this);
}

Profiler::~Profiler()
{
    end_time = get_time();

    duration = end_time - start_time;

    add_profile(this);
}
