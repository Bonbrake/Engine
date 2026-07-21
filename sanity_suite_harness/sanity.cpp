#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

// Copies of minimal skeleton types so the suite never pollutes engine headers.
struct Entity { uint32_t id; };
struct Transform { float x,y,z,w; };
struct MeshComponent { uint32_t handle; };

// Mirrors M0-EXT-01 AlignToCacheLine behaviour in standalone form.
template <typename T>
requires std::is_trivially_copyable_v<T> && alignof(T) >= alignof(std::max_align_t)
struct alignas(64) AlignToCacheLine {
    alignas(64) T value;
    constexpr const T* operator->() const { return &value; }
    constexpr T* operator->*() { return &value; }
};

template <typename T>
concept Hashable = requires(T t) { std::hash<T>{}(t); };

template <typename Value>
struct alignas(64) Bucket {
    Value value{}; // explicit init here: "Bucket() : value{} {}" is unavailable in aggregate C++20 shorthand
    bool occupied = false;
};

template <Hashable Key, typename Value>
class alignas(64) FlatHashMapping {
public:
    Value* insert_or_get(const Key& k, bool* inserted_out) {
        *inserted_out = false;
        for (auto& b : buckets_) {
            if (!b.occupied && inserted_out) { *inserted_out = true; b.occupied = true; b.value = Value{}; return &b.value; }
        }
        return nullptr;
    }
    bool contains(const Key& k) const {
        (void)k; return false;
    }
    constexpr static std::size_t bucket_count() noexcept { return 128u; }
private:
    Bucket<Value> buckets_[128];
};

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <Arithmetic T>
constexpr T clamp(T v, T lo, T hi) {
    return (v < lo) ? lo : (v > hi ? hi : v);
}

// Alternate SPSC ring-buffer idiom used by grid/chunk handoffs and sanity-suite reuse boundary.
template <typename T, std::size_t N>
class SPSCRing {
public:
    bool push(const T& v) {
        std::size_t next = (head_ + 1u) % N;
        if (next == tail_) return false;
        buf_[head_] = v;
        head_ = next;
        return true;
    }
    bool pop(T& out) {
        if (tail_ == head_) return false;
        out = buf_[tail_];
        tail_ = (tail_ + 1u) % N;
        return true;
    }
    bool empty() const { return tail_ == head_; }
private:
    T buf_[N]{};
    std::size_t head_ = 0;
    std::size_t tail_ = 0;
};

// Minimal mock collision/failure case for WFC recovery analogue.
enum class ConflictResult : uint32_t { NoConflict = 0, Single = 1, Double = 2 };

constexpr ConflictResult pick_conflict(uint32_t k) {
    if (k == 0u) return ConflictResult::NoConflict;
    if (k < 0x40000000u) return ConflictResult::Single;
    return ConflictResult::Double;
}

struct SaintVenantFlux { float left=0, right=0, top=0, bottom=0; };

constexpr float stencil_sum(const SaintVenantFlux& f) {
    return f.left + f.right + f.top + f.bottom;
}

} // namespace

int main() {
    // 1. cache-line alignment
    {
        AlignToCacheLine<Transform> slot;
        static_assert(sizeof(decltype(slot)) >= 64);
        static_assert(alignof(decltype(slot)) >= 64);
        slot->x = 1.0f; slot->y = 2.0f; slot->z = 3.0f; slot->w = 0.0f;
        assert(slot->x == 1.0f);
    }

    // 2. SPSC push/pop / exhaustion edge
    {
        SPSCRing<uint32_t, 4> ring;
        assert(ring.push(10));
        assert(ring.push(20));
        assert(ring.push(30));
        uint32_t out=0;
        assert(ring.pop(out) && out==10);
        assert(ring.pop(out) && out==20);
        // FIFO preserved?
        assert(ring.push(40));
        assert(ring.pop(out) && out==30);
        assert(ring.pop(out) && out==40);
        assert(ring.empty());
    }

    // 3. SPSC push beyond capacity returns false
    {
        SPSCRing<uint32_t, 4> ring;
        assert(ring.push(1));
        assert(ring.push(2));
        assert(ring.push(3));
        assert(!ring.push(4)); // full
    }

    // 4. FlatHashMapping push then reuse / insert-or-get flow
    {
        FlatHashMapping<uint32_t, uint64_t> map;
        bool inserted=false;
        uint64_t* a = map.insert_or_get(7u, &inserted);
        assert(a != nullptr);
        assert(inserted);
        bool inserted2=false;
        uint64_t* b = map.insert_or_get(7u, &inserted2);
        assert(b != nullptr);
        assert(!inserted2);
        (void)b;
    }

    // 5. clamp boundaries
    {
        assert(clamp<int>(-1, 0, 100) == 0);
        assert(clamp<int>(120, 0, 100) == 100);
        assert(clamp<int>(50, 0, 100) == 50);
    }

    // 6. WFC-style conflict mapping determinism
    {
        std::vector<uint32_t> seeds = {0u, 1u, 0x40000000u, 0x7fffffffu};
        std::vector<ConflictResult> out;
        for (uint32_t s : seeds) out.push_back(pick_conflict(s));
        assert(out[0] == ConflictResult::NoConflict);
        assert(out[1] == ConflictResult::Single);
        assert(out[2] == ConflictResult::Single);
        assert(out[3] == ConflictResult::Double);
    }

    // 7. Saint-Venant flux stencil shape + neutral equilibrium
    {
        SaintVenantFlux f{0.1f, 0.1f, 0.0f, 0.0f};
        assert(std::fabs(stencil_sum(f)) < 1e-5f);
        f = SaintVenantFlux{1.0f, 2.0f, 3.0f, 4.0f};
        assert(stencil_sum(f) == 10.0f);
    }

    return 0;
}
