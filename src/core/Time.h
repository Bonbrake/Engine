#pragma once

#include <chrono>

namespace core {

constexpr double TIME_STEP = 1.0 / 60.0;

class Time {
public:
    static void init() {
        last_time_ = std::chrono::high_resolution_clock::now();
        accumulator_ = 0.0;
    }

    static void update() {
        auto current_time = std::chrono::high_resolution_clock::now();
        double frame_time = std::chrono::duration<double>(current_time - last_time_).count();
        last_time_ = current_time;
        update(frame_time);
    }

    // Overload for injecting deterministic delta times in tests
    static void update(double frame_time) {
        if (frame_time > 0.25)
            frame_time = 0.25;

        accumulator_ += frame_time;
    }

    static bool consume_fixed_step() {
        if (accumulator_ >= TIME_STEP) {
            accumulator_ -= TIME_STEP;
            return true;
        }
        return false;
    }

private:
    static inline std::chrono::time_point<std::chrono::high_resolution_clock> last_time_;
    static inline double accumulator_{0.0};
};

} // namespace core