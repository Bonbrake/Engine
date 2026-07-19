// [M0-EXT-25] Per-Fiber Exception Containment for enkiTS Jobs - regression test.
//
// A task whose ExecuteRange throws must NOT terminate the enkiTS worker thread
// or the host process. Every ExecuteRange override in the engine now wraps its
// body in try/catch (Engine.cpp SpscSweeperTask, PhysicsSystem.cpp
// AsyncCollisionBaker, RenderGraph.cpp PassTask). This test locks the invariant
// the guards depend on: after a contained throw, the scheduler is still usable.
#include <catch2/catch_test_macros.hpp>
#include <enkiTS/TaskScheduler.h>
#include <atomic>
#include <stdexcept>

namespace {

struct ThrowingTask : enki::ITaskSet {
    std::atomic<bool> contained = false;
    void ExecuteRange(enki::TaskSetPartition, uint32_t) override {
        try {
            throw std::runtime_error("contained");
        } catch (const std::exception&) {
            contained = true;   // mirrors the engine guard's catch block
        }
    }
};

struct NormalTask : enki::ITaskSet {
    std::atomic<bool> ran = false;
    void ExecuteRange(enki::TaskSetPartition, uint32_t) override { ran = true; }
};

} // namespace

TEST_CASE("M0-EXT-25 enkiTS ExecuteRange exceptions are contained", "[m0][ext25]") {
    enki::TaskScheduler ts;
    ts.Initialize();  // void-return; in-process scheduler init always succeeds

    ThrowingTask bad;
    ts.AddTaskSetToPipe(&bad);
    ts.WaitforTask(&bad);
    REQUIRE(bad.contained);     // guard swallowed the exception

    // Scheduler must remain usable after a contained throw.
    NormalTask ok;
    ts.AddTaskSetToPipe(&ok);
    ts.WaitforTask(&ok);
    REQUIRE(ok.ran);            // worker thread survived
}
