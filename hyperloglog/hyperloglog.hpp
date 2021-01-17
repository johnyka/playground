#ifndef HYPERLOGLOG_HPP
#define HYPERLOGLOG_HPP

#include <functional>
#include <vector>
#include <math.h>
#include <iostream>

inline unsigned long long unaligned_load(const char *p) {
    unsigned long long result;
    __builtin_memcpy(&result, p, sizeof(result));
    return result;
}

// Loads n bytes, where 1 <= n < 8.
inline unsigned long long load_bytes(const char *p, int n) {
    unsigned long long result = 0;
    --n;
    do
        result = (result << 8) + static_cast<unsigned char>(p[n]);
    while (--n >= 0);
    return result;
}

inline unsigned long long shift_mix(unsigned long long v) {
    return v ^ (v >> 47);
}
// Implementation of Murmur hash for 64-bit unsigned long long.
unsigned long long my_Hash_bytes(const void *ptr, unsigned long long len, unsigned long long seed) {
    static const unsigned long long mul =
        (((unsigned long long)0xc6a4a793ULL) << 32UL) + (unsigned long long)0x5bd1e995ULL;
    const char *const buf = static_cast<const char *>(ptr);

    // Remove the bytes not divisible by the sizeof(unsigned long long).  This
    // allows the main loop to process the data as 64-bit integers.
    const unsigned long long len_aligned = len & ~(unsigned long long)0x7;
    const char *const end = buf + len_aligned;
    unsigned long long hash = seed ^ (len * mul);
    for (const char *p = buf; p != end; p += 8) {
        const unsigned long long data = shift_mix(unaligned_load(p) * mul) * mul;
        hash ^= data;
        hash *= mul;
    }
    if ((len & 0x7) != 0) {
        const unsigned long long data = load_bytes(end, len & 0x7);
        hash ^= data;
        hash *= mul;
    }
    hash = shift_mix(hash) * mul;
    hash = shift_mix(hash);
    return hash;
}

template <typename T> unsigned long long myhash(const T &value) {
    unsigned long long __clength = sizeof(value);
    unsigned long long __seed = 0xc70f6907ULL;
    return value != 0.0 ? my_Hash_bytes(&value, __clength, __seed) : 0;
}

template <typename ForwardIterator>
size_t hyperloglog(ForwardIterator begin, ForwardIterator end) {
    std::vector<int> buckets(pow(2,14), 0);

    for(auto it = begin; it != end; ++it){
        unsigned long long hash = myhash(*it);
        unsigned long index = hash >> 50;
        int zeroCount = __builtin_clzl((hash << 14));

        if(zeroCount > buckets[index]){
            buckets[index] = zeroCount;
        }

    }

    double sum = 0;
    for( auto i : buckets) {
        sum += pow(2,-(i+1));
    }

    return pow(sum,-1) * (0.72134)*pow(pow(2,14),2);
}

#endif /* ifndef HYPERLOGLOG_HPP */

