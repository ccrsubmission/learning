#include <schad/infrastructure/infrastructure_impl.h>
#include "infrastructure.h"

auto schad::create_infastructure(
        InfrastructureParameters const& params,
        RewardFunction const& reward,
        vector<shared_ptr<Policy>> const& policies,
        learning::LearningMethodFactory const& learning,
        World * world,
        StatsCollector * stat_collector) 
        -> unique_ptr<Infrastructure> {
    auto const num_arms = policies.size();

    vector<unique_ptr<PolicyInstance>> policies_inst{};
    for (auto const& p : policies) {
        policies_inst.emplace_back(
            p->instantiate(world, params.buffer_size()));
    }

    return make_unique<InfrastructureImpl>(
        params, std::move(policies_inst), 
        learning.instantiate(world->rng(), num_arms),
        reward, stat_collector
    );
}
