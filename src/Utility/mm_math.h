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

template <size_t N>
struct vec
{
    // Our data
    float v[N];

    // Constructors
    template <class ...Args>
    constexpr vec(Args&& ...args)
    {

    }

    // Accessors
    float& operator [](int i)
    {
        return v[i];
    }
};

template <class T, size_t N>
constexpr double dot ( const T<N>& a, const T<N>& b) noexcept
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

template <class T>
constexpr void normalize( const T& v ) noexcept
{
    const float mag = sqrt(v.x*v.x + v.y*v.y);
    return { v.x / mag, v.y / mag };
}