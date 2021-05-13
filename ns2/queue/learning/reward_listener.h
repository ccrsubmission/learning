#ifndef NS_REWARD_LISTENER_H
#define NS_REWARD_LISTENER_H

#include <vector>

struct RewardListener {
    virtual void report_rewards(
            size_t arm_idx, 
            double reward, 
            std::vector<double> const& subrewards) = 0;

    RewardListener() = default;
    RewardListener(RewardListener const&) = delete;
    RewardListener& operator=(RewardListener const&) = delete;

    virtual ~RewardListener() = default;
};

#endif // NS_REWARD_LISTENER_H
