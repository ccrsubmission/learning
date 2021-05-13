#include "weighted_throughput_reward.h"

namespace {
using namespace schad;

class WeightedThroughputReward : public RewardFunction {
public:
    WeightedThroughputReward() 
        : total_arrival_{}, total_transmission_{} {
        reset({});
    }

    void note_transmission(uint32_t time, Packet const& p) override {
        if (is_ready(p) && !is_expired(time, p)) {
            total_transmission_ += p.value();
        }
    }

    void note_arrival(Packet const& p) override {
        total_arrival_ += p.value();
    }

    auto get() const -> Reward override {
        return Reward{total_transmission_, total_arrival_};
    }

    void reset(vector<Packet const *> const& buffer) override {
        total_transmission_ = total_arrival_ = 0.0;
        for (auto p : buffer) {
            total_arrival_ += p->value();
        }
    }

    auto clone() const -> unique_ptr<RewardFunction> override {
        return make_unique<WeightedThroughputReward>();
    }

    void to_json(json& j) const override {
        j = {{"type", "weighted_throughput"}};
    }

private:
    double total_arrival_;
    double total_transmission_;
};

}

auto schad::rewards::weighted_throughput() 
        -> unique_ptr<RewardFunction const> {
    return make_unique<WeightedThroughputReward>();
}
