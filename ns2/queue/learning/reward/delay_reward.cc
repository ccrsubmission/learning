#include "delay_reward.h"

#include <string>

static class DelayRewardClass : public TclClass {
public:
    DelayRewardClass() : TclClass("Reward/Delay") {
    }

    TclObject *create(int argc, const char *const *argv) override {
        if (argc >= 5) {
            return new DelayReward{stof(argv[4])};
        }

        return nullptr;
    }

} delay_reward_class;

DelayReward::DelayReward(double scale) 
    : current_average_{0.0}, count_{0}, scale_{scale} { 
}

void DelayReward::note_transmission(Packet const * p) {
    auto const now = Scheduler::instance().clock();
    auto const delay = now - HDR_CMN(p)->timestamp();

    current_average_ += (delay - current_average_) / (count_ + 1);
    count_++;
}

auto DelayReward::get_value() const -> double {
    // TODO: make scaling adjustable
    return std::max(0.0, 1.0 - current_average_ * scale_);
}

void DelayReward::reset(vector<Packet const*>packets) {
    // TODO: maybe somehow take into account that packets were already 
    // residing in a buffer?
    count_ = 0;
    current_average_ = 0.0;
}

auto DelayReward::clone() const -> unique_ptr<Reward> {
    return make_unique<DelayReward>(scale_);
}
