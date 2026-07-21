// [M0-EXT-08] Vulkan Descriptor Indexing Bindless Storage Handle Page Allocator
//
// Implementation — bitmask free-list with round-robin page hinting.

#include "ze/render/DescriptorSlotAllocator.h"
#include "ze/core/Logger.h"

namespace render {

DescriptorSlotAllocator::DescriptorSlotAllocator()
    : nextPageHint_(0)
    , usedCount_(0)
{
    for (auto& page : pages_) {
        page.freeMask = ~uint64_t(0);   // all bits 1 = all slots free
    }
}

DescriptorSlot DescriptorSlotAllocator::Allocate() {
    for (uint32_t i = 0; i < kMaxPages; ++i) {
        uint32_t pageIdx = (nextPageHint_ + i) % kMaxPages;
        Page& page = pages_[pageIdx];

        if (page.freeMask == 0) {
            continue;   // page full, try next
        }

        // find first free slot via trailing-zero-bit count
        uint32_t slotIdx = static_cast<uint32_t>(std::countr_zero(page.freeMask));

        // allocate: clear the bit
        page.freeMask &= ~(uint64_t(1) << slotIdx);

        nextPageHint_ = (pageIdx + 1) % kMaxPages;
        ++usedCount_;

        DescriptorSlot slot;
        slot.pageIndex = pageIdx;
        slot.slotIndex = slotIdx;
        slot.handle    = (pageIdx << 6) | slotIdx;
        return slot;
    }

    // all pages exhausted
    LOG_ERROR("DescriptorSlotAllocator: out of slots (used={})", usedCount_);
    return DescriptorSlot::Invalid();
}

void DescriptorSlotAllocator::Free(DescriptorSlot slot) {
    assert(slot.pageIndex < kMaxPages);
    assert(slot.slotIndex < kSlotsPerPage);

    Page& page   = pages_[slot.pageIndex];
    uint64_t bit = uint64_t(1) << slot.slotIndex;

    // double-free guard
    assert((page.freeMask & bit) == 0 && "double-free detected");

    page.freeMask |= bit;
    --usedCount_;
}

bool DescriptorSlotAllocator::IsValid(DescriptorSlot slot) const {
    if (slot.pageIndex >= kMaxPages || slot.slotIndex >= kSlotsPerPage) {
        return false;
    }
    // a slot is currently allocated if its bit is CLEAR
    return (pages_[slot.pageIndex].freeMask & (uint64_t(1) << slot.slotIndex)) == 0;
}

} // namespace render