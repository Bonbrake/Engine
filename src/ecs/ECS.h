#pragma once

#include <entt/entt.hpp>
#include <atomic>
#include <vector>
#include <memory>

#include <thread>

namespace ecs {

struct DeferredMutation {
    entt::entity targetEntity;
    uint32_t operationBitmask; // For example: 1 = add component A, 2 = remove component B, 4 = destroy entity, etc.
};

// [M1-EXT-06] EnTT Archetype Component SPSC Mutation Queue Committer
struct alignas(64) SPSCMutationQueue {
    DeferredMutation dataPool[1024];
    alignas(64) std::atomic<uint32_t> writeHead{0};
    alignas(64) std::atomic<uint32_t> readHead{0};

    inline bool PushMutation(entt::entity ent, uint32_t op) {
        uint32_t currWrite = writeHead.load(std::memory_order_relaxed);
        uint32_t retryCount = 0;
        while (((currWrite + 1) & 1023) == readHead.load(std::memory_order_acquire)) {
            std::this_thread::yield();
            if (++retryCount > 100000) {
                return false; // Ring buffer full, timed out
            }
        }
        dataPool[currWrite] = { ent, op };
        writeHead.store((currWrite + 1) & 1023, std::memory_order_release);
        return true;
    }

    // Called once at the start of each main-thread tick, before any system runs.
    inline void DrainToRegistry(entt::registry& registry, void (*apply)(entt::registry&, const DeferredMutation&)) {
        uint32_t currWrite = writeHead.load(std::memory_order_acquire);
        uint32_t currRead = readHead.load(std::memory_order_relaxed);
        while (currRead != currWrite) {
            apply(registry, dataPool[currRead]);
            currRead = (currRead + 1) & 1023;
        }
        readHead.store(currRead, std::memory_order_release);
    }
};

struct WorkerSnapshotBuffers {
    std::vector<uint8_t*> rawData;
    std::vector<uint32_t> sparseSet;
};

class ECSContext {
public:
        ECSContext(uint32_t numWorkerThreads) : numWorkers(numWorkerThreads) {
        if (numWorkers == 0) numWorkers = 1;
        workerQueues = std::make_unique<SPSCMutationQueue[]>(numWorkers);
        workerSnapshotBuffers.resize(numWorkers);
    }

    entt::registry& GetRegistry() { return registry; }
    SPSCMutationQueue& GetWorkerQueue(uint32_t threadIndex) { return workerQueues[threadIndex % numWorkers]; }
    WorkerSnapshotBuffers& GetWorkerSnapshotBuffers(uint32_t threadIndex) { return workerSnapshotBuffers[threadIndex % numWorkers]; }

    // Drains all queues safely on the main thread
    void DrainMutations(void (*apply)(entt::registry&, const DeferredMutation&)) {
        for (uint32_t i = 0; i < numWorkers; ++i) {
            workerQueues[i].DrainToRegistry(registry, apply);
        }
    }

private:
    entt::registry registry;
    // One SPSC queue per worker thread
    std::unique_ptr<SPSCMutationQueue[]> workerQueues;
    std::vector<WorkerSnapshotBuffers> workerSnapshotBuffers;
    uint32_t numWorkers;
};

} // namespace ecs
