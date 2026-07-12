#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <thread>
#include <vector>
#include "../src/core/PRNG.h"
#include "../src/core/Memory.h"
#include "../src/core/FileHandleRing.h"

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

TEST_CASE("FileHandleRing - Single Handle", "[filehandlering]") {
    core::FileHandleRing ring;
    core::OSFileHandle h1 = reinterpret_cast<core::OSFileHandle>(0x1234);
    ring.handles.push_back(h1);

    REQUIRE(core::AcquireHandle(ring) == h1);
    REQUIRE(core::AcquireHandle(ring) == h1);
    REQUIRE(core::AcquireHandle(ring) == h1);
}

TEST_CASE("FileHandleRing - Multi-Handle Round Robin", "[filehandlering]") {
    core::FileHandleRing ring;
    core::OSFileHandle h1 = reinterpret_cast<core::OSFileHandle>(0x1000);
    core::OSFileHandle h2 = reinterpret_cast<core::OSFileHandle>(0x2000);
    core::OSFileHandle h3 = reinterpret_cast<core::OSFileHandle>(0x3000);

    ring.handles.push_back(h1);
    ring.handles.push_back(h2);
    ring.handles.push_back(h3);

    REQUIRE(core::AcquireHandle(ring) == h1);
    REQUIRE(core::AcquireHandle(ring) == h2);
    REQUIRE(core::AcquireHandle(ring) == h3);
    REQUIRE(core::AcquireHandle(ring) == h1); // Wraparound
}

TEST_CASE("FileHandleRing - Cursor Overflow Wraparound", "[filehandlering]") {
    core::FileHandleRing ring;
    core::OSFileHandle h1 = reinterpret_cast<core::OSFileHandle>(0x1000);
    core::OSFileHandle h2 = reinterpret_cast<core::OSFileHandle>(0x2000);
    core::OSFileHandle h3 = reinterpret_cast<core::OSFileHandle>(0x3000);
    core::OSFileHandle h4 = reinterpret_cast<core::OSFileHandle>(0x4000);

    ring.handles.push_back(h1);
    ring.handles.push_back(h2);
    ring.handles.push_back(h3);
    ring.handles.push_back(h4);

    ring.cursor = UINT32_MAX - 1;

    REQUIRE(core::AcquireHandle(ring) == h3);
    REQUIRE(core::AcquireHandle(ring) == h4);
    REQUIRE(core::AcquireHandle(ring) == h1);
    REQUIRE(core::AcquireHandle(ring) == h2);
}

TEST_CASE("FileHandleRing - Concurrency", "[filehandlering]") {
    core::FileHandleRing ring;
    const int NUM_HANDLES = 4;
    for (int i = 0; i < NUM_HANDLES; i++) {
        ring.handles.push_back(reinterpret_cast<core::OSFileHandle>(static_cast<uintptr_t>(0x1000 + i * 0x1000)));
    }

    const int NUM_THREADS = 8;
    const int ITERATIONS_PER_THREAD = 10000;

    std::atomic<int> counts[NUM_HANDLES]{0, 0, 0, 0};

    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.emplace_back([&]() {
            for (int j = 0; j < ITERATIONS_PER_THREAD; j++) {
                core::OSFileHandle h = core::AcquireHandle(ring);
                uintptr_t val = reinterpret_cast<uintptr_t>(h);
                int index = (val - 0x1000) / 0x1000;
                counts[index].fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    int expected_count = (NUM_THREADS * ITERATIONS_PER_THREAD) / NUM_HANDLES;

    for (int i = 0; i < NUM_HANDLES; i++) {
        REQUIRE(counts[i].load() == expected_count);
    }
}
