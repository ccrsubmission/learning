#ifndef REWARD_H
#define REWARD_H

#include <schad/packet/packet.h>

namespace schad {

struct Reward {
    Reward(double value, optional<double> total=std::nullopt)
        : value_{value}, total_{total} {
    }

    auto value() const {
        return value_;
    }

    auto total() const {
        return total_;
    }

private:
    double value_;
    optional<double> total_;
};

inline auto maybe_value(optional<Reward> const& v) -> optional<double> {
    if (v.has_value()) {
        return v->value();
    } else {
        return std::nullopt;
    }
}

}

#endif // REWARD_H
