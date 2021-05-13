#ifndef UCB_PARAMETERS_H
#define UCB_PARAMETERS_H

#include <schad/learning/average.h>

namespace schad::learning {

struct UCBParameters {
    UCBParameters() : average_{KeepAll{}}, ksi_{2.0}, 
        restricted_exploration_{false} { 
    }

    auto& set_ksi(double ksi) {
        ksi_ = ksi;
        return *this;
    }

    auto ksi() const {
        return ksi_;
    }

    auto& set_average(Average average) {
        average_ = average;
        return *this;
    }

    auto average() const {
        return average_;
    }

    auto& set_restricted_exploration(bool restricted_exploration) {
        restricted_exploration_ = restricted_exploration;
        return *this;
    }

    auto restricted_exploration() const {
        return restricted_exploration_;
    }

private:
    Average average_;
    double ksi_;
    bool restricted_exploration_;
};

inline void to_json(json& j, UCBParameters const& p) {
    j = {{"ksi", p.ksi()},
        {"average", p.average()},
        {"restricted_exploration", p.restricted_exploration()}};
}

}

#endif // UCB_PARAMETERS_H
