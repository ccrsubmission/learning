#ifndef POISSON_SOURCE_PARAMETERS_H
#define POISSON_SOURCE_PARAMETERS_H

#include <schad/common.h>

namespace schad {

class PoissonSourceParameters {
public:
    PoissonSourceParameters(double rate = 1.0)
        : rate_{rate}, min_value_{1.0}, max_value_{1.0},
          min_work_{1}, max_work_{1}, 
          min_slack_{}, max_slack_{} {
    }

    auto& set_min_value(double min_value) {
        min_value_ = min_value;
        return *this;
    }

    auto min_value() const {
        return min_value_;
    }

    auto& set_max_value(double max_value) {
        max_value_ = max_value;
        return *this;
    }

    auto max_value() const {
        return max_value_;
    }

    auto& set_min_work(uint32_t min_work) {
        min_work_ = min_work;
        return *this;
    }

    auto min_work() const {
        return min_work_;
    }

    auto& set_max_work(uint32_t max_work) {
        max_work_ = max_work;
        return *this;
    }

    auto max_work() const {
        return max_work_;
    }

    auto& set_rate(double rate) {
        rate_ = rate;
        return *this;
    }

    auto rate() const {
        return rate_;
    }

    auto& set_min_slack(double min_slack) {
        min_slack_ = min_slack;
        return *this;
    }

    auto min_slack() const {
        return min_slack_;
    }

    auto& set_max_slack(double max_slack) {
        max_slack_ = max_slack;
        return *this;
    }

    auto max_slack() const {
        return max_slack_;
    }

private:
    double rate_;
    double min_value_;
    double max_value_;
    uint32_t min_work_;
    uint32_t max_work_;
    optional<double> min_slack_;
    optional<double> max_slack_;
};

inline void to_json(json& j, PoissonSourceParameters const& params) {
    j = {
        {"rate", params.rate()},
        {"min_value", params.min_value()},
        {"max_value", params.max_value()},
        {"min_work", params.min_work()},
        {"max_work", params.max_work()}
    };
    if (params.min_slack()) {
        j["slack"] = {
            {"min", *params.min_slack()},
            {"max", *params.max_slack()}
        };
    }
}

} // namespace schad_main

#endif
