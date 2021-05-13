#ifndef NS_THROUGHPUT_REWARD_H
#define NS_THROUGHPUT_REWARD_H

#include "reward.h"

class ThroughputReward : public Reward {
public:
    ThroughputReward(double minimal_delay, double maximal_delay, double scale);

    void note_transmission(Packet const * p) override;

    auto get_value() const -> double override;

    void reset(vector<Packet const*> packets) override;

    auto clone() const -> unique_ptr<Reward> override;

private:
    double const minimal_delay_;
    double const maximal_delay_;
    double const scale_;
    double current_total_;
};


#endif //NS_THROUGHPUT_REWARD_H
