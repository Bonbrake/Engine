#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cstdint>
#include <stdexcept>

// Ext systems headers
#include "ze/ecs/SpatialHash.h"
#include "ze/render/CommandPoolMatrix.h"
#include "ze/render/FrameDeletionQueue.h"
#include "ze/render/StagingRingBuffer.h"
#include "ze/core/ThreadAffinity.h"

// ---------------------------------------------------------
// [M1-EXT-01] SpatialHash Uniform Grid Bucketing
// ---------------------------------------------------------
TEST_CASE("SpatialHashKey Euclidean Modulo Math", "[M1-EXT-01]") {
    // Tests for proper flooring of negative numbers (std::floor)
    // Positive
    uint64_t k1 = ecs::SpatialHashKey(3.5f, 5.0f); // cx=1, cz=2
    REQUIRE(k1 == ((1ULL << 32) | 2ULL));

    // Negative coordinates should floor downwards (-0.5 / 2 = -0.25 -> -1)
    uint64_t k2 = ecs::SpatialHashKey(-0.5f, -2.5f); // cx=-1, cz=-2
    int64_t expectedCx = -1;
    int64_t expectedCz = -2;
    uint64_t expectedKey = (static_cast<uint64_t>(expectedCx) << 32) | (static_cast<uint64_t>(expectedCz) & 0xFFFFFFFFull);
    REQUIRE(k2 == expectedKey);
}

// ---------------------------------------------------------
// [M1-EXT-08] Spatial Hash Quadtree Subdivision bit-packing
// ---------------------------------------------------------
TEST_CASE("ComputeSubdividedCellKey Bit Packing", "[M1-EXT-08]") {
    uint64_t parentKey = 0xABCD1234;
    // For subX = 2, subZ = 3
    // leafOffset = ((2 & 3) << 2) | (3 & 3) = (2 << 2) | 3 = 8 | 3 = 11 (0xB)
    uint64_t childKey = ecs::ComputeSubdividedCellKey(parentKey, 2, 3);
    
    REQUIRE(childKey == ((parentKey << 4) | 0x0Bull));
}

// ---------------------------------------------------------
// [M1-EXT-03] CommandPoolMatrix
// ---------------------------------------------------------
TEST_CASE("CommandPoolMatrix Index Mapping", "[M1-EXT-03]") {
    render::CommandPoolMatrix matrix;
    // Configure layout only (no Vulkan device needed) to test the pure index math.
    matrix.ConfigureLayout(3, 4); // 3 frames, 4 threads

    // Valid bounds
    REQUIRE(matrix.GetPoolIndex(0, 0) == 0);
    REQUIRE(matrix.GetPoolIndex(0, 3) == 3);
    REQUIRE(matrix.GetPoolIndex(1, 0) == 4);
    REQUIRE(matrix.GetPoolIndex(2, 2) == 10);

    // Out of bounds / assert case (Death test equivalent via exception)
    REQUIRE_THROWS_AS(matrix.GetPoolIndex(0, 4), std::out_of_range);
    REQUIRE_THROWS_AS(matrix.GetPoolIndex(1, 5), std::out_of_range);
}

TEST_CASE("CommandPoolMatrix Initialize rejects null device", "[M1-EXT-03]") {
    render::CommandPoolMatrix matrix;
    // A null VkDevice would null-deref inside the Vulkan loader (dispatch table read);
    // Initialize must reject it up front rather than crash.
    REQUIRE_THROWS_AS(matrix.Initialize(VK_NULL_HANDLE, 0, 3, 4), std::invalid_argument);
}

// ---------------------------------------------------------
// [M1-EXT-04] Frame-Scoped Deletion Queue
// ---------------------------------------------------------
TEST_CASE("Frame-Scoped Deletion Queue Boundary", "[M1-EXT-04]") {
    render::FrameDeletionQueue queue;
    int destroyCount = 0;
    auto dtor = [&destroyCount]() { destroyCount++; };

    // Enqueued on frame fence 100
    queue.Push(dtor, 100);
    
    // Boundary checks:
    queue.Flush(99);
    REQUIRE(destroyCount == 0); // Not ready
    
    // Exact boundary case: Current == EnqueuedFence
    queue.Flush(100);
    REQUIRE(destroyCount == 1); // Triggers exactly when completed == enqueued fence

    // Check it removed it
    queue.Flush(101);
    REQUIRE(destroyCount == 1); // Still 1

    // Push another for FlushAll
    queue.Push(dtor, 200);
    queue.FlushAll();
    REQUIRE(destroyCount == 2);
}

// ---------------------------------------------------------
// [M1-EXT-05] Persistent-Mapped Staging Ring Buffer
// ---------------------------------------------------------
TEST_CASE("Staging Ring Buffer Math", "[M1-EXT-05]") {
    render::StagingRingBuffer rb;
    // We just test the math, passing VK_NULL_HANDLE
    // Initialize(allocator, totalPoolSize)
    rb.Initialize(VK_NULL_HANDLE, 1024);

    REQUIRE(rb.GetHead() == 0);
    REQUIRE(rb.GetTail() == 0);
    REQUIRE(rb.GetUsedCount() == 0);

    // 1. Allocate normal
    size_t offset1 = rb.Allocate(256);
    REQUIRE(offset1 == 0);
    REQUIRE(rb.GetHead() == 256);
    REQUIRE(rb.GetUsedCount() == 256);

    // 2. Disambiguation strategy test: head == tail (empty vs full)
    // Let's fill it exactly to capacity (1024 bytes)
    size_t offset2 = rb.Allocate(1024 - 256);
    REQUIRE(offset2 == 256);
    REQUIRE(rb.GetHead() == 0); // Wraps around
    REQUIRE(rb.GetTail() == 0); 
    REQUIRE(rb.GetUsedCount() == 1024); // Full! head == tail

    // At this point head == tail. If we didn't have usedCount, we wouldn't know if empty or full.
    // Ensure we can't allocate more when full
    REQUIRE_THROWS_AS(rb.Allocate(1), std::out_of_range);

    // 3. Free some space
    rb.Free(256); // Tail moves to 256
    REQUIRE(rb.GetTail() == 256);
    REQUIRE(rb.GetUsedCount() == 1024 - 256);

    // Ensure we can allocate again since there's room
    size_t offset3 = rb.Allocate(128);
    REQUIRE(offset3 == 0); // Allocated at head
    REQUIRE(rb.GetHead() == 128);
    REQUIRE(rb.GetUsedCount() == 1024 - 256 + 128);
    
    // Ensure we catch freeing more than used
    REQUIRE_THROWS_AS(rb.Free(2000), std::out_of_range);

    // Free remaining allocations to return to empty
    rb.Free(1024 - 256); // frees offset 256..1024
    REQUIRE(rb.GetTail() == 0);
    rb.Free(128);        // frees offset 0..128
    REQUIRE(rb.GetTail() == 128);
    REQUIRE(rb.GetHead() == 128);
    REQUIRE(rb.GetUsedCount() == 0);
}

TEST_CASE("Staging Ring Buffer Wrap and Padding Zero-Leak", "[M1-EXT-05]") {
    render::StagingRingBuffer rb;
    rb.Initialize(VK_NULL_HANDLE, 1024);

    // 1. Allocate 512 bytes at 0
    size_t off1 = rb.Allocate(512);
    REQUIRE(off1 == 0);
    REQUIRE(rb.GetHead() == 512);

    // 2. Allocate 384 bytes at 512 (leaves 128 bytes at end: [896, 1024))
    size_t off2 = rb.Allocate(384);
    REQUIRE(off2 == 512);
    REQUIRE(rb.GetHead() == 896);
    REQUIRE(rb.GetUsedCount() == 896);

    // 3. Free first block (512) -> tail moves to 512
    rb.Free(512);
    REQUIRE(rb.GetTail() == 512);
    REQUIRE(rb.GetUsedCount() == 384);

    // 4. Allocate 256 bytes -> does not fit in remaining 128 bytes at end!
    // Must wrap to 0, wasting 128 bytes at end as padding.
    size_t off3 = rb.Allocate(256);
    REQUIRE(off3 == 0);
    REQUIRE(rb.GetHead() == 256);
    REQUIRE(rb.GetWrapPoint() == 896);
    // usedCount = 384 active + 128 wasted + 256 active = 768
    REQUIRE(rb.GetUsedCount() == 768);

    // 5. Free second block (384) -> reaches wrapPoint (512 + 384 = 896)
    // Must reclaim the 128 wasted padding bytes and wrap tail to 0!
    rb.Free(384);
    REQUIRE(rb.GetTail() == 0);
    REQUIRE(rb.GetWrapPoint() == 0);
    REQUIRE(rb.GetUsedCount() == 256); // Exactly only the 256 bytes of off3 remain!

    // 6. Free third block (256) -> buffer should be 100% empty with ZERO phantom bytes!
    rb.Free(256);
    REQUIRE(rb.GetTail() == 256);
    REQUIRE(rb.GetHead() == 256);
    REQUIRE(rb.GetUsedCount() == 0);

    // 7. Verify multiple wrap cycles run indefinitely without leaking
    for (int cycle = 0; cycle < 10; ++cycle) {
        size_t a = rb.Allocate(640);
        REQUIRE(a % 64 == 0);
        rb.Free(640);
        REQUIRE(rb.GetUsedCount() == 0);
    }
}

// ---------------------------------------------------------
// [M0-EXT-12] Thread-Affinity Bitmask Allocator
// ---------------------------------------------------------
TEST_CASE("Thread-Affinity Bitmask Allocator", "[M0-EXT-12]") {
    core::ThreadAffinityAllocator allocator;
    uint64_t mask1 = allocator.AllocateBitmask();
    uint64_t mask2 = allocator.AllocateBitmask();

    REQUIRE(mask1 > 0);
    REQUIRE((mask1 & (mask1 - 1)) == 0);
    
    REQUIRE(mask2 > 0);
    REQUIRE((mask2 & (mask2 - 1)) == 0);
}
