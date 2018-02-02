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
    std::unique_lock<std::mutex> lock(queue_mutex); // Lock the buffer
    buffer.emplace_back(item);                      // Add the item
  }

  void modify() noexcept {
    cond.wait([]{ return  });
  }

  void done() noexcept {
    cond.notify_one();
  }

  T operator[](std::uint32_t index) const noexcept {
    return buffer[index];
  }

  T at(std::uint32_t index) const {
    if (index > size) throw "Out of bounds";
    else return buffer[index];
  }

  buffer.at(0) = particle3;
};
