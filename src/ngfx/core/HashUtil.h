//this code is adopted from https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0814r0.pdf
#pragma once

namespace ngfx {
    struct HashUtil {
        template <typename T>
        static inline void combine(size_t& seed, const T& val) {
            seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        template<typename... Types>
        static inline size_t combine(const Types&... args)
        {
            size_t seed = 0;
            (combine(seed, args), ...);
            return seed;
        }
    };
}
