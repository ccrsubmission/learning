#include "reward_config.h"
#include <schad/reward/weighted_throughput_reward.h>

auto schad::load_reward(json const& cfg) -> unique_ptr<RewardFunction const> {
    if (cfg.at("type") == "weighted_throughput") {
        return schad::rewards::weighted_throughput();
    } else {
        throw std::invalid_argument(
            "Unknown reward type: " + cfg.at("type").get<string>()
        );
    }
}
