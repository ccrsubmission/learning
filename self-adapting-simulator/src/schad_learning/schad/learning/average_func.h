#ifndef AVERAGE_FUNC_H
#define AVERAGE_FUNC_H

#include <schad/learning/average.h>
#include <queue>

namespace schad::learning {

template<class Average, bool squares = false> struct AverageFunc;

template<bool squares>
struct AverageFunc<SlidingWindow, squares> {
    explicit AverageFunc(SlidingWindow const& params) 
        : params_{params}, history_{}, current_value_{0.0}, current_count_{0} {
    }

    void operator+=(optional<double> value) {
        history_.push(value);

        if (value.has_value()) {
            if constexpr (squares) {
                current_value_ += *value * *value;
            } else {
                current_value_ += *value;
            }
            current_count_ += 1;
        }

        if (history_.size() > params_.num_steps()) {
            if (history_.front().has_value()) {
                if constexpr (squares) {
                    current_value_ -= *history_.front() * *history_.front();
                } else {
                    current_value_ -= *history_.front();
                }
                current_count_ -= 1;
            }
            history_.pop();
        }
    }

    auto avg() const -> double {
        return current_value_ / (count() + 0.001);
    }

    auto count() const -> double {
        return current_count_;
    }

private:
    SlidingWindow const params_;
    std::queue<optional<double>> history_;
    double current_value_;
    size_t current_count_;
};

template<bool squares>
struct AverageFunc<Exponential, squares> {
    explicit AverageFunc(Exponential const& params)
        : params_{params}, current_value_{0.0}, current_count_{0.0} {
    }

    void operator+=(optional<double> value) {
        if constexpr (squares) {
            current_value_ *= params_.gamma() * params_.gamma();
            current_value_ += value.value_or(0.0) * value.value_or(0.0);
        } else {
            current_value_ *= params_.gamma();
            current_value_ += value.value_or(0.0);
        }
        current_count_ *= params_.gamma();
        current_count_ += (value.has_value() ? 1.0 : 0.0); 
    }

    auto avg() const -> double {
        return current_value_ / (count() + 0.001);
    }

    auto count() const -> double {
        return current_count_;
    }

private:
    Exponential const params_;
    double current_value_;
    double current_count_;
};

template<bool squares>
struct AverageFunc<KeepAll, squares> {
    explicit AverageFunc(KeepAll const&) : current_value_{0.0}, current_count_{0} {
    }

    void operator+=(optional<double> value) {
        if (value.has_value()) {
            if constexpr(squares) {
                current_value_ += 
                    (*value * *value - current_value_) / double(current_count_ + 1);
            } else {
                current_value_ += 
                    (*value - current_value_) / double(current_count_ + 1);
            }
            current_count_ += 1;
        }
    }

    auto avg() const -> double {
        return current_value_;
    }

    auto count() const -> double {
        return current_count_;
    }

private:
    double current_value_;
    size_t current_count_;
};


}

#endif // AVERAGE_FUNC_H
