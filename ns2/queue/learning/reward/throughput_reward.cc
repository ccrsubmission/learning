#include "throughput_reward.h"

#include <string>

static class ThroughputRewardClass : public TclClass {
public:
    ThroughputRewardClass() : TclClass("Reward/Throughput") {

    }

    TclObject *create(int argc, const char *const *argv) override {
        if (argc >= 7) {
            return new ThroughputReward{stof(argv[4]), stof(argv[5]), stof(argv[6])};
        }

        return nullptr;
    }
} throughput_reward_class;


ThroughputReward::ThroughputReward(double minimal_delay, double maximal_delay, double scale)
: minimal_delay_{minimal_delay}, maximal_delay_{maximal_delay}, scale_{scale}, current_total_{0.0} {

}

void ThroughputReward::note_transmission(Packet const * p) {
    auto const now = Scheduler::instance().clock();
    auto const delay = now - HDR_CMN(p)->timestamp();
    if (delay < maximal_delay_) {
        if (minimal_delay_ >= maximal_delay_) {
            current_total_ += HDR_CMN(p)->size();
        } else {
            auto const penalty = (delay - minimal_delay_) / (maximal_delay_  - minimal_delay_);
            auto const clamped_penalty = min(1.0, max(0.0, penalty));
            current_total_ += HDR_CMN(p)->size() * (1.0 - clamped_penalty);
        }
    }
}

auto ThroughputReward::get_value() const -> double {
    return current_total_ * scale_;
}

void ThroughputReward::reset([[maybe_unused]] vector<const Packet *> packets) {
    current_total_ = 0.;
}

auto ThroughputReward::clone() const -> unique_ptr<Reward> {
    return make_unique<ThroughputReward>(minimal_delay_, maximal_delay_, scale_);
}
