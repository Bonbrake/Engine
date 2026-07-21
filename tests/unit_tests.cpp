#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "ze/core/PRNG.h"
#include "ze/core/Memory.h"

TEST_CASE("PRNG Determinism", "[prng]") {
    core::prng::SplitMix64 rng1(12345);
    core::prng::SplitMix64 rng2(12345);

    REQUIRE(rng1.next() == rng2.next());
    REQUIRE(rng1.next() == rng2.next());
}

TEST_CASE("Cache Line Alignment", "[memory]") {
    REQUIRE(core::memory::AlignToCacheLine(0) == 0);
    REQUIRE(core::memory::AlignToCacheLine(1) == 64);
    REQUIRE(core::memory::AlignToCacheLine(63) == 64);
    REQUIRE(core::memory::AlignToCacheLine(64) == 64);
    REQUIRE(core::memory::AlignToCacheLine(65) == 128);
}
