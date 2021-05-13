#ifndef NS_INTERVAL_PARAMS_H
#define NS_INTERVAL_PARAMS_H

struct IntervalParams {
    IntervalParams(double interval, size_t measure_delay, size_t num_subintervals)
        : interval_{interval}
        , measure_delay_{measure_delay}
        , num_subintervals_{num_subintervals} {}

    auto interval() const {
        return interval_;
    }

    auto subinterval() const {
        return interval() / num_subintervals_;
    }

    auto num_subintervals() const {
        return num_subintervals_;
    }

    auto is_switch_interval(size_t interval_idx) const -> bool {
        return interval_idx % (measure_delay_ + 1) == 0;
    }

private:
    double interval_;
    size_t measure_delay_;
    size_t num_subintervals_;
};

#endif // NS_INTERVAL_PARAMS_H
