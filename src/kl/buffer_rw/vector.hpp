#pragma once

#include "kl/buffer_rw.hpp"

#include <vector>

namespace kl {

template <typename T>
kl::buffer_reader& operator>>(kl::buffer_reader& r, std::vector<T>& vec)
{
    const auto size = r.read<std::uint32_t>();

    vec.clear();
    vec.reserve(size);

    for (std::uint32_t i = 0; i < size; ++i)
    {
        vec.push_back(r.read<T>());
        if (r.err())
        {
            vec.clear();
            break;
        }
    }

    return r;
}
} // namespace kl
