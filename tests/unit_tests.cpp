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

TEST_CASE("WorkerSubAllocator basic operations", "[memory]") {
    core::memory::WorkerSubAllocator allocator;

    // Must be able to create a 1MB virtual block
    allocator.Create(1024 * 1024);

    SECTION("Allocate and Free") {
        auto alloc1 = allocator.Allocate(128);
        REQUIRE(alloc1.has_value());
        REQUIRE(alloc1->offset % 64 == 0);

        auto alloc2 = allocator.Allocate(256);
        REQUIRE(alloc2.has_value());
        REQUIRE(alloc2->offset % 64 == 0);
        REQUIRE(alloc1->offset != alloc2->offset);

        allocator.Free(alloc1->handle);
        allocator.Free(alloc2->handle);
    }

    SECTION("Out of Memory") {
        // Try to allocate more than the block size
        auto large_alloc = allocator.Allocate(2 * 1024 * 1024);
        REQUIRE(!large_alloc.has_value());
    }

    SECTION("Zero Size Allocation") {
        auto zero_alloc = allocator.Allocate(0);
        REQUIRE(!zero_alloc.has_value());
    }
}
