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

#include <chrono>
#include <iostream>
#include "../src/ecs/EnTTCache.h"

TEST_CASE("EnTTCache Benchmark", "[benchmark][ecs]") {
    entt::registry reg;
    struct DummyComponent { float x, y, z; };

    for (int i = 0; i < 50000; i++) {
        reg.emplace<DummyComponent>(reg.create(), 1.0f, 2.0f, 3.0f);
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<uint8_t*> localRawData;
    std::vector<uint32_t> localSparseSet;
    // Simulate 1000 snapshots (e.g. 1000 frames)
    for (int i = 0; i < 1000; i++) {
        auto cache = ecs::SnapshotComponentPool<DummyComponent>(reg, localRawData, localSparseSet);
        // Do something trivial so it isn't optimized out
        auto* comp = cache.ResolveComponentPointerDirect<DummyComponent>(static_cast<uint32_t>(100));
        REQUIRE(comp != nullptr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "\n[BENCHMARK] EnTTCache snapshot (1000 iterations, 50k entities): " << ms << " ms\n";
    // We expect the original to take a fair amount of time due to allocating 50k elements twice per iteration.
}
