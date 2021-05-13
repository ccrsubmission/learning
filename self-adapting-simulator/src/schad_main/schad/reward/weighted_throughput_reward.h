#ifndef WEIGHTED_THROUGHPUT_REWARD_H
#define WEIGHTED_THROUGHPUT_REWARD_H

#include <schad/common.h>
#include <schad/reward/reward_function.h>

namespace schad::rewards {

auto weighted_throughput() -> unique_ptr<RewardFunction const>;

}

#endif // WEIGHTED_THROUGHPUT_REWARD_H
