#pragma once

#include <enkiTS/TaskScheduler.h>
#include <memory>

namespace core {

class JobSystem {
public:
    static void init(uint32_t numThreads = 0);
    static void shutdown();
    static enki::TaskScheduler* get() { return scheduler_.get(); }

private:
    static std::unique_ptr<enki::TaskScheduler> scheduler_;
};

} // namespace core
