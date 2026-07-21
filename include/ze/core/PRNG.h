#pragma once

#include <cstdint>

namespace core {
namespace prng {

// SplitMix64 thread isolation
// Thread-Local SplitMix64 PRNG Seed Distributor
class SplitMix64 {
public:
    explicit SplitMix64(uint64_t seed = 0) : state_(seed) {}

    uint64_t next() {
        state_ += 0x9e3779b97f4a7c15;
        uint64_t z = state_;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
        z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
        return z ^ (z >> 31);
    }

private:
    uint64_t state_;
};

// Global thread-local instance
inline thread_local SplitMix64 tls_rng;

// Helper to seed the thread local RNG
inline void seed_thread_local(uint64_t seed) {
    tls_rng = SplitMix64(seed);
}

// Helper to get next random
inline uint64_t next_random() {
    return tls_rng.next();
}

} // namespace prng
} // namespace core
