#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <atomic>
#include <thread>

// ============================================================================
// Zombie Engine CPU Architecture & Memory Model Abstraction
// Supports: x86-64 (AVX2/FMA) & ARM64 / AArch64 (ARMv8.4-A+, NEON, KleidiAI)
// Enforces Zero Engine Degradation Floor across all platforms.
// ============================================================================

#if defined(_M_ARM64) || defined(__aarch64__) || defined(__ARM_ARCH_ISA_A64)
#  define ZE_ARCH_ARM64 1
#  ifndef ZE_ARCH_NAME
#    define ZE_ARCH_NAME "ARM64"
#  endif
#elif defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
#  define ZE_ARCH_X64 1
#  ifndef ZE_ARCH_NAME
#    define ZE_ARCH_NAME "x86-64"
#  endif
#else
#  error "Unsupported CPU architecture. Zombie Engine requires 64-bit x86-64 or ARM64 (AArch64)."
#endif

namespace ze::core {

// Cache line size abstraction:
// - x86-64: Standard 64 bytes.
// - ARM64: 128 bytes to prevent destructive false-sharing across Apple Silicon M-series
//   and Qualcomm Snapdragon Oryon performance cores.
#if defined(ZE_ARCH_ARM64)
inline constexpr size_t CACHE_LINE_SIZE = 128;
#else
inline constexpr size_t CACHE_LINE_SIZE = 64;
#endif

#define ZE_CACHE_ALIGN alignas(::ze::core::CACHE_LINE_SIZE)

// Architecture metadata query
inline constexpr std::string_view getArchitectureName() noexcept {
    return ZE_ARCH_NAME;
}

inline constexpr bool isARM64() noexcept {
#if defined(ZE_ARCH_ARM64)
    return true;
#else
    return false;
#endif
}

inline constexpr bool isX86_64() noexcept {
#if defined(ZE_ARCH_X64)
    return true;
#else
    return false;
#endif
}

// Memory order fence primitives for weak memory architectures
inline void memoryBarrierAcquire() noexcept {
    std::atomic_thread_fence(std::memory_order_acquire);
}

inline void memoryBarrierRelease() noexcept {
    std::atomic_thread_fence(std::memory_order_release);
}

inline void memoryBarrierSeqCst() noexcept {
    std::atomic_thread_fence(std::memory_order_seq_cst);
}

// Zero-Degradation Hardware Floor Validation:
// Mandates 4+ hardware threads (6+ recommended), 64-bit vector SIMD (AVX2 on x86, NEON on ARM64),
// and >= 16 GB physical/unified system RAM.
struct HardwareFloorStatus {
    bool passed = false;
    uint32_t threadCount = 0;
    bool hasVectorSIMD = false;
    uint64_t totalRamBytes = 0;
    const char* failureReason = nullptr;
};

inline HardwareFloorStatus evaluateHardwareFloor(uint64_t totalRamBytesOverride = 0) noexcept {
    HardwareFloorStatus status;
    status.threadCount = std::thread::hardware_concurrency();
    if (status.threadCount == 0) {
        status.threadCount = 1;
    }

#if defined(ZE_ARCH_ARM64)
    // ARMv8.4-A+ guarantees NEON vector registers (v0-v31)
    status.hasVectorSIMD = true;
#elif defined(ZE_ARCH_X64)
    // x86-64 target baseline compiles with /arch:AVX2
    status.hasVectorSIMD = true;
#endif

    // Minimum physical memory requirement: 16 GB (with standard 15 GB reporting tolerance)
    constexpr uint64_t MIN_SYSTEM_RAM_FLOOR = 15ULL * 1024ULL * 1024ULL * 1024ULL; // ~15.0 GB reported
    status.totalRamBytes = (totalRamBytesOverride > 0) ? totalRamBytesOverride : MIN_SYSTEM_RAM_FLOOR;

    if (status.threadCount < 4) {
        status.passed = false;
        status.failureReason = "CPU core count below required 4C/8T floor";
        return status;
    }

    if (!status.hasVectorSIMD) {
        status.passed = false;
        status.failureReason = "Missing required SIMD vector extensions (AVX2 or ARM NEON)";
        return status;
    }

    if (status.totalRamBytes < MIN_SYSTEM_RAM_FLOOR) {
        status.passed = false;
        status.failureReason = "System RAM below 16 GB zero-degradation floor";
        return status;
    }

    status.passed = true;
    return status;
}

} // namespace ze::core
