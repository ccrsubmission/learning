#ifndef INFRASTRUCTURE_IMPL_H
#define INFRASTRUCTURE_IMPL_H

#include <schad/infrastructure/infrastructure.h>

namespace schad {

class InfrastructureImpl : public Infrastructure {
public:
    InfrastructureImpl(
        InfrastructureParameters const& params,
        vector<unique_ptr<PolicyInstance>> policies,
        unique_ptr<learning::LearningMethod> learning,
        RewardFunction const& reward,
        StatsCollector * collector
    );

    auto active_policy() const -> tuple<PolicyInstance *, RewardFunction *> override;
    auto simulated_policies() const 
        -> vector<tuple<PolicyInstance *, RewardFunction *>> override;

    void tick() override;

private:
    void change_current(size_t new_policy_idx);
    void reset_all_rewards();
    void finish_batch();
    void start_batch();

private:
    InfrastructureParameters const params_;

    vector<unique_ptr<PolicyInstance>> const policies_;
    unique_ptr<learning::LearningMethod> const learning_;
    vector<unique_ptr<RewardFunction>> rewards_pool_;

    StatsCollector * const stat_collector_;

    size_t active_policy_idx_;
    vector<size_t> simulated_policies_idxs_;
    size_t since_last_batch_;
};

}

#endif // INFRASTRUCTURE_IMPL_H 
