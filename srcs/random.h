#ifndef RANDOM_H
#define RANDOM_H
#include <cstdint>

// SplitMix64: private state per worker; no shared rand() state or lock.
inline std::uint64_t& random_state() {
    static thread_local std::uint64_t state = 0;
    return state;
}
inline std::uint64_t mix_random_bits(std::uint64_t z) {
    z = (z ^ (z >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94d049bb133111eb);
    return z ^ (z >> 31);
}
inline void seed_random(std::uint64_t seed) {
    // Hash pixel seeds so neighbours do not start at adjacent points in one stream.
    random_state() = mix_random_bits(seed);
}
inline double random_double() {
    const auto z = mix_random_bits(random_state() += UINT64_C(0x9e3779b97f4a7c15));
    return static_cast<double>(z >> 11) * 0x1.0p-53;
}
inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}
#endif
