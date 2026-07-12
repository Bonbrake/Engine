#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "../src/core/PRNG.h"
#include "../src/core/Memory.h"

TEST_CASE("PRNG Determinism", "[prng]") {
    core::prng::SplitMix64 rng1(12345);
    core::prng::SplitMix64 rng2(12345);

    REQUIRE(rng1.next() == rng2.next());
    REQUIRE(rng1.next() == rng2.next());
}

TEST_CASE("Cache Line Alignment", "[memory]") {
    // Note: The power-of-two and overflow preconditions are asserted in AlignSize,
    // but they are intentionally untested via death-test here because standard asserts
    // crash the test runner. We rely on these passing valid cases.
    REQUIRE(core::memory::AlignSize(0) == 0);
    REQUIRE(core::memory::AlignSize(1) == 64);
    REQUIRE(core::memory::AlignSize(63) == 64);
    REQUIRE(core::memory::AlignSize(64) == 64);
    REQUIRE(core::memory::AlignSize(65) == 128);

    // Test generalized alignment
    REQUIRE(core::memory::AlignSize(10, 16) == 16);
    REQUIRE(core::memory::AlignSize(31, 32) == 32);
    REQUIRE(core::memory::AlignSize(32, 32) == 32);

    // Near SIZE_MAX boundary
    REQUIRE(core::memory::AlignSize(SIZE_MAX - 64) == SIZE_MAX - 63);
    REQUIRE(core::memory::AlignSize(SIZE_MAX - 63) == SIZE_MAX - 63);
}
