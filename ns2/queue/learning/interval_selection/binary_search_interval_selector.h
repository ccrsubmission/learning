#ifndef NS_BINARY_SEARCH_INTERVAL_SELECTOR_H
#define NS_BINARY_SEARCH_INTERVAL_SELECTOR_H

#include "interval_selector.h"
#include "utils/order_correlator.h"

class BinarySearchIntervalSelector : public IntervalSelector {
public:
    BinarySearchIntervalSelector(
        double initial_interval, 
        size_t min_samples, 
        double min_threshold
        );

    void report_rewards(size_t                   arm_idx, 
                        double                    reward, 
                        vector<double> const& subrewards) override;

    void reset(size_t num_arms) override;
    auto take_new_params() -> optional<IntervalParams> override;

private:
    double const initial_interval_;
    size_t const min_samples_;
    double const min_threshold_;

    vector<OrderCorrelator> correlations_;
    optional<double> next_interval_;
    double current_inteval_;
    optional<double> last_worst_;
    bool done_;
};

#endif
