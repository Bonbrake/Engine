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
    REQUIRE(core::memory::AlignToCacheLine(0) == 0);
    REQUIRE(core::memory::AlignToCacheLine(1) == 64);
    REQUIRE(core::memory::AlignToCacheLine(63) == 64);
    REQUIRE(core::memory::AlignToCacheLine(64) == 64);
    REQUIRE(core::memory::AlignToCacheLine(65) == 128);
}

#include "../src/core/Time.h"

TEST_CASE("Time Subsystem", "[time]") {
    SECTION("Time Accumulation and Fixed Step Consumption") {
        core::Time::init();

        // Inject exactly one fixed time step (1/60s)
        core::Time::update(core::TIME_STEP);

        REQUIRE(core::Time::consume_fixed_step() == true);
        REQUIRE(core::Time::consume_fixed_step() == false); // Accumulator should be empty
    }

    SECTION("Time Cap (0.25s clamp)") {
        core::Time::init();

        // Inject a very large time jump (e.g. 1.0 seconds)
        core::Time::update(1.0);

        // It should be capped at 0.25s.
        // 0.25 / (1.0 / 60.0) = 15 fixed steps exactly
        for (int i = 0; i < 15; ++i) {
            REQUIRE(core::Time::consume_fixed_step() == true);
        }

        // The 16th should fail since the accumulator was clamped to 0.25s
        REQUIRE(core::Time::consume_fixed_step() == false);
    }

    SECTION("Multiple partial accumulations") {
        core::Time::init();

        // Half a step shouldn't trigger consumption
        core::Time::update(core::TIME_STEP * 0.5);
        REQUIRE(core::Time::consume_fixed_step() == false);

        // Another half step should make it reach the threshold
        core::Time::update(core::TIME_STEP * 0.5);
        REQUIRE(core::Time::consume_fixed_step() == true);
        REQUIRE(core::Time::consume_fixed_step() == false);
    }
}
