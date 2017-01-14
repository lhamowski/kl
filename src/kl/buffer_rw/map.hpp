#pragma once

#include "kl/buffer_rw.hpp"
#include "kl/buffer_rw/pair.hpp"

#include <map>

namespace kl {

template <typename K, typename V>
kl::buffer_reader& operator>>(kl::buffer_reader& r, std::map<K, V>& map)
{
    const auto size = r.read<std::uint32_t>();
    map.clear();

    for (std::uint32_t i = 0; i < size; ++i)
    {
        // NB: We can't use pair<const K, V> here
        map.insert(r.read<std::pair<K, V>>());
        if (r.err())
        {
            map.clear();
            break;
        }
    }

    return r;
}
} // namespace kl
