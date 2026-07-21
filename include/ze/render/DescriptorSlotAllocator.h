#pragma once

#include <array>
#include <bit>
#include <cassert>
#include <cstdint>

// [M0-EXT-08] Vulkan Descriptor Indexing Bindless Storage Handle Page Allocator
//
// A non-fragmenting bitmask free-list allocator. Each page tracks 64 slots via a
// bitmask; assigning a slot does a single bit-scan for the next free bit instead
// of searching a table. Backs every bindless material-index lookup — M1's ECS
// material indexing and [M4.5-EXT-09]'s per-primitive MaterialIdx both resolve
// through this.

namespace render {

struct DescriptorSlot {
    uint32_t pageIndex;
    uint32_t slotIndex;     // 0-63 within page
    uint32_t handle;        // packed: (pageIndex << 6) | slotIndex

    bool IsValid() const { return handle != ~0u; }

    static DescriptorSlot Invalid() { return {0, 0, ~0u}; }
};

class DescriptorSlotAllocator {
public:
    static constexpr uint32_t kSlotsPerPage = 64;
    static constexpr uint32_t kMaxPages    = 16;   // 1024 total slots
    static constexpr uint32_t kTotalSlots  = kMaxPages * kSlotsPerPage;

    DescriptorSlotAllocator();

    DescriptorSlot Allocate();
    void          Free(DescriptorSlot slot);
    bool          IsValid(DescriptorSlot slot) const;
    bool          IsFull()   const { return usedCount_ >= kTotalSlots; }
    bool          IsEmpty()  const { return usedCount_ == 0; }
    uint32_t      UsedCount()   const { return usedCount_; }
    uint32_t      FreeCount()   const { return kTotalSlots - usedCount_; }
    uint32_t      Capacity()    const { return kTotalSlots; }

private:
    struct Page {
        uint64_t freeMask;  // 1 = free, 0 = allocated
    };

    std::array<Page, kMaxPages> pages_;
    uint32_t    nextPageHint_;  // round-robin hint, not a hard partition
    uint32_t    usedCount_;
};

} // namespace render
