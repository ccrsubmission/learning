#ifndef AVERAGE_H
#define AVERAGE_H

#include <schad/common.h>
#include <variant>

namespace schad::learning {

struct SlidingWindow {
    explicit SlidingWindow(size_t num_steps) : num_steps_{num_steps} { }

    auto num_steps() const {
        return num_steps_;
    }

private:
    size_t num_steps_;
};

inline void to_json(json& j, SlidingWindow const& sw) {
    j = {{"type", "sliding_window"},
        {"num_steps", sw.num_steps()}};
}

struct Exponential {
    explicit Exponential(double gamma) : gamma_{gamma} { }

    auto gamma() const {
        return gamma_;
    }

private:
    double gamma_;
};

inline void to_json(json& j, Exponential const& e) {
    j = {{"type", "exponential"},
        {"gamma", e.gamma()}};
}

struct KeepAll {};

inline void to_json(json& j, KeepAll const&) {
    j = {{"type", "keep_all"}};
}

using Average = std::variant<KeepAll, SlidingWindow, Exponential>;

inline void to_json(json& j, Average const& avg) {
    std::visit([&j] (auto const& avg) {
        to_json(j, avg);
    }, avg);
}

}

#endif // AVERAGE_H
