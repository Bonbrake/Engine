#include <catch2/catch.hpp>
#include "../src/ecs/GenerationalTable.h"
#include <string>

// A simple structure to test with GenerationalTable
struct TestItem {
    int value = 0;
    std::string name = "";

    bool operator==(const TestItem& other) const {
        return value == other.value && name == other.name;
    }
};

TEST_CASE("GenerationalTable Basic Operations", "[generational_table]") {
    ecs::GenerationalTable<TestItem> table;

    SECTION("Insert and Get return valid data") {
        ecs::Handle h1 = table.Insert({42, "Item1"});
        REQUIRE(h1.index == 0);
        REQUIRE(h1.generation == 1);

        TestItem* item1 = table.Get(h1);
        REQUIRE(item1 != nullptr);
        REQUIRE(item1->value == 42);
        REQUIRE(item1->name == "Item1");
    }

    SECTION("Remove clears data and increments generation") {
        ecs::Handle h1 = table.Insert({42, "Item1"});
        table.Remove(h1);

        // Access via stale handle should fail
        TestItem* staleItem = table.Get(h1);
        REQUIRE(staleItem == nullptr);
        REQUIRE_FALSE(ecs::IsHandleValid(h1, table.generations));
    }
}

TEST_CASE("GenerationalTable Slot Reuse", "[generational_table]") {
    ecs::GenerationalTable<TestItem> table;

    ecs::Handle h1 = table.Insert({10, "First"});
    table.Remove(h1);

    SECTION("Inserting after removal reuses index but increments generation") {
        ecs::Handle h2 = table.Insert({20, "Second"});

        // Should reuse the same index
        REQUIRE(h2.index == h1.index);
        // Generation should be incremented to 2
        REQUIRE(h2.generation == 2);
        // Generation is strictly greater than the old one
        REQUIRE(h2.generation > h1.generation);

        // Verify the old handle is still invalid
        REQUIRE_FALSE(ecs::IsHandleValid(h1, table.generations));
        REQUIRE(table.Get(h1) == nullptr);

        // Verify the new handle gets the new data
        TestItem* item2 = table.Get(h2);
        REQUIRE(item2 != nullptr);
        REQUIRE(item2->value == 20);
    }
}

TEST_CASE("GenerationalTable Edge Cases", "[generational_table]") {
    ecs::GenerationalTable<TestItem> table;

    SECTION("Removing an already-removed handle is safe") {
        ecs::Handle h1 = table.Insert({100, "Target"});
        table.Remove(h1);

        // This should not crash or cause undefined behavior
        REQUIRE_NOTHROW(table.Remove(h1));

        // The free list should only have one entry for this index
        // Even if we removed it twice, IsHandleValid check inside Remove prevents double-freeing
        REQUIRE(table.freeIndices.size() == 1);
        REQUIRE(table.freeIndices[0] == h1.index);
    }

    SECTION("Out-of-bounds index gracefully fails validation") {
        ecs::Handle h1 = table.Insert({1, "Test"});

        // Create a garbage handle with an index that is completely out of bounds
        ecs::Handle outOfBoundsHandle;
        outOfBoundsHandle.index = 9999;
        outOfBoundsHandle.generation = 1;

        // Validation should fail safely
        REQUIRE_FALSE(ecs::IsHandleValid(outOfBoundsHandle, table.generations));
        REQUIRE(table.Get(outOfBoundsHandle) == nullptr);

        // Removing an out of bounds handle should also be safe and do nothing
        REQUIRE_NOTHROW(table.Remove(outOfBoundsHandle));
    }
}

/*
 * Note on generation overflow:
 * The `generation` field is a uint32_t. If an entity slot is reused 2^32 times,
 * the generation counter will wrap back around to 0 or 1. If a user happens to hold a very
 * old handle with that exact same generation number and index, it will incorrectly be
 * considered valid. This is theoretically possible but highly unlikely in standard use,
 * and currently unhandled by the implementation.
 */
