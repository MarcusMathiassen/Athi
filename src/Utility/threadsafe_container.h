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

#include <vector>
#include <mutex>
#include <memory>
namespace ThreadSafe
{
template <class T, class A = std::allocator<T> >
class vector {
public:

    std::vector<T, A> buffer;
    std::mutex buffer_mutex;

    typedef A allocator_type;
    typedef typename A::value_type value_type;
    typedef typename A::reference reference;
    typedef typename A::const_reference const_reference;
    typedef typename A::difference_type difference_type;
    typedef typename A::size_type size_type;

    class iterator {
    public:
        typedef typename A::difference_type difference_type;
        typedef typename A::value_type value_type;
        typedef typename A::reference reference;
        typedef typename A::pointer pointer;
    };
    class const_iterator {
    public:
        typedef typename A::difference_type difference_type;
        typedef typename A::value_type value_type;
    };

    iterator begin() noexcept
    {
      return buffer.begin();
    }

    const_iterator begin() const noexcept
    {
      return buffer.begin();
    }

    iterator end() noexcept
    {
      return buffer.end();
    }

    const_iterator end() const noexcept
    {
      return buffer.end();
    }

    template<class ...Args>
    void emplace_back(Args&&... args) noexcept
    {
      std::unique_lock<std::mutex> lock(buffer_mutex);
      buffer.emplace_back(std::forward<Args>(args)...);
    }

    reference operator[](size_type i) noexcept
    {
      return buffer[i];
    }
    const_reference operator[](size_type i) const noexcept
    {
      return buffer[i];
    }
    size_type size() const noexcept
    {
      return buffer.size();
    }
    bool empty() const noexcept
    {
      return buffer.empty();
    }
    void clear() noexcept
    {
      std::unique_lock<std::mutex> lock(buffer_mutex);
      return buffer.clear();
    }

    void lock() noexcept
    {
      buffer_mutex.lock();
    }

    void unlock() noexcept
    {
      buffer_mutex.unlock();
    }
};
}; // namespace ThreadSafe
