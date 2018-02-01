#pragma once

#include <vector>
#include <condition_variable>
#include <mutex>

template <class T>
struct threadsafe_container
{
  std::vector<T> buffer;
  std::size_t size;
  std::condition_variable cond;
  std::mutex buffer_mutex;

  void add(const T& item) noexcept {

    // Lock the buffer
    std::unique_lock<std::mutex> lock(queue_mutex);

    // Add the item
    buffer.emplace_back(item);
  }

  // Returns a copy of the 
  T operator[](std::uint32_t index) const noexcept {
    return buffer[index];
  }

  T at(std::uint32_t index) const {
    if (index > size) throw "Out of bounds";
    else return buffer[index];
  }

  buffer.at(0) = particle3;
};
