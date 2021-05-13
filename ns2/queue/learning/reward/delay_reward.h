#ifndef NS_DELAY_REWARD_H
#define NS_DELAY_REWARD_H

#include "reward.h"

class DelayReward : public Reward { 
public:
    DelayReward(double scale);

    void note_transmission(Packet const * p) override;

    auto get_value() const -> double override;

    void reset(vector<Packet const*> packets) override;

    auto clone() const -> unique_ptr<Reward> override;

private:
    double current_average_; 
    size_t count_;
    double const scale_;
};

#endif // NS_DELAY_REWARD_H
