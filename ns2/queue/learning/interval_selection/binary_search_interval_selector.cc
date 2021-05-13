#include "binary_search_interval_selector.h"

#include <algorithm>
#include <numeric>
#include <cassert>
#include <iostream>

static class BinarySearchIntervalSelectorClass : public TclClass {
public:
    BinarySearchIntervalSelectorClass(): TclClass("IntervalSelector/BinarySearch") { }

    TclObject *create(int argc, const char *const *argv) override {
        if (argc >= 7) {
            return new BinarySearchIntervalSelector{
                    stod(argv[4]), stoul(argv[5]), stod(argv[6])
            };
        }
        return nullptr;
    }
} binary_search_interval_selector_class;

BinarySearchIntervalSelector::BinarySearchIntervalSelector(
    double initial_interval, 
    size_t min_samples, 
    double min_threshold
    ) 
    : initial_interval_{initial_interval}
    , min_samples_{min_samples}
    , min_threshold_{min_threshold}
    , last_worst_{}
    , done_{false}
{}

void BinarySearchIntervalSelector::report_rewards(
        size_t arm_idx, double reward, vector<double> const& subrewards
        ) {
    if (next_interval_ || done_) {
        return;
    }
    correlations_[arm_idx].add_sample(
        reward, 
        std::accumulate(begin(subrewards), end(subrewards), 0.0) 
            / (double) subrewards.size()
        );
    size_t const num_ready = std::count_if(
            begin(correlations_), end(correlations_),
            [] (auto const& x) { return x.get_result(); });
    if (num_ready != correlations_.size()) {
        return;
    }
    auto const worst = *std::min_element(
            begin(correlations_), end(correlations_),
            [] (auto const& x, auto const& y) {
                return x.get_result() < y.get_result();
            })->get_result();
    if ((!last_worst_ || worst < *last_worst_) && worst >= min_threshold_) {

        last_worst_ = worst;
        next_interval_ = current_inteval_ / 2;
        std::cerr << "WHOOA: " << *next_interval_ << "s with corr = " << worst << "\n";
    } else {
        done_ = true;
    }
}

void BinarySearchIntervalSelector::reset(size_t num_arms) {
    correlations_ = vector(num_arms, OrderCorrelator(min_samples_));
    next_interval_ = initial_interval_;
}

auto BinarySearchIntervalSelector::take_new_params() -> optional<IntervalParams> {
    if (next_interval_) {
        auto const result = IntervalParams(*next_interval_, 0, 2);
        current_inteval_ = *next_interval_;
        next_interval_ = nullopt;
        correlations_ = vector(correlations_.size(), OrderCorrelator(min_samples_));
        return result;
    } else {
        return nullopt;
    }
}
