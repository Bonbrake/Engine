#include "JobSystem.h"
#include "Logger.h"

namespace core {

std::unique_ptr<enki::TaskScheduler> JobSystem::scheduler_;

void JobSystem::init(uint32_t numThreads) {
    scheduler_ = std::make_unique<enki::TaskScheduler>();
    if (numThreads > 0) {
        scheduler_->Initialize(numThreads);
    } else {
        scheduler_->Initialize();
    }
    LOG_INFO("JobSystem initialized with {} threads", scheduler_->GetNumTaskThreads());
}

void JobSystem::shutdown() {
    scheduler_.reset();
    LOG_INFO("JobSystem shut down");
}

} // namespace core
