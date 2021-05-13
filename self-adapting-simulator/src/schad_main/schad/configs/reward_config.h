#ifndef REWARD_CONFIG_H
#define REWARD_CONFIG_H

#include <schad/reward/reward_function.h>

namespace schad {

auto load_reward(json const& cfg) -> unique_ptr<RewardFunction const>;

}

#endif // REWARD_CONFIG_H
