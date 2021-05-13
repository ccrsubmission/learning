#include "fixed_interval_selector.h"

class FixedIntervalSelectorClass : public TclClass {
public:
    FixedIntervalSelectorClass() : TclClass("IntervalSelector/Fixed") {}

    TclObject * create(int argc, const char * const * argv) override {
        if (argc >= 7) {
            return new FixedIntevalSelector{
                stod(argv[4]), stoul(argv[5]), stoul(argv[6])
            };
        }
        return nullptr;
    }

} class_fixed_interval_selector;

FixedIntevalSelector::FixedIntevalSelector(
        double interval, size_t measure_delay, size_t num_subintervals)
    : params_(interval, measure_delay, num_subintervals)
    
    , has_been_taken_{false}
{}

FixedIntevalSelector::~FixedIntevalSelector() {
}


void FixedIntevalSelector::report_rewards(
        size_t arm_idx, 
        double reward, 
        std::vector<double> const& subrewards) {}

void FixedIntevalSelector::reset(size_t num_arms) {
    has_been_taken_ = false;
}

auto FixedIntevalSelector::take_new_params() -> optional<IntervalParams> {
    if (!has_been_taken_) {
        has_been_taken_ = true;
        return params_;
    } else {
        return nullopt;
    }
}
