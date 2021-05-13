#ifndef NS_FIXED_INTERVAL_SELECTOR_H
#define NS_FIXED_INTERVAL_SELECTOR_H

#include "interval_selection/interval_selector.h"

class FixedIntevalSelector : public IntervalSelector {
public:
    FixedIntevalSelector(double interval, 
                         size_t measure_delay, 
                         size_t num_subintervals);
    ~FixedIntevalSelector();

    void report_rewards(size_t                   arm_idx, 
                        double                    reward, 
                        std::vector<double> const& subrewards) override;

    auto take_new_params() -> optional<IntervalParams> override;

    void reset(size_t num_arms) override;

private:
    IntervalParams const params_;
    bool has_been_taken_;
};


#endif // NS_FIXED_INTERVAL_SELECTOR_H
