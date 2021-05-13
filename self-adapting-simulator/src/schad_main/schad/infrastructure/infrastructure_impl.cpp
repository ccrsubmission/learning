#include "infrastructure_impl.h"
#include <iterator>

namespace schad {

InfrastructureImpl::InfrastructureImpl(
    InfrastructureParameters const& params,
    vector<unique_ptr<PolicyInstance>> policies,
    unique_ptr<learning::LearningMethod> learning,
    RewardFunction const& reward,
    StatsCollector * collector)
    : params_{params},
      policies_{std::move(policies)}, learning_{std::move(learning)},
      rewards_pool_(params.num_simulated() + 1),
      stat_collector_{collector},
      active_policy_idx_{0}, simulated_policies_idxs_{},
      since_last_batch_{0}  {
    std::generate(begin(rewards_pool_), end(rewards_pool_),
            [&reward] { return reward.clone(); });
    start_batch();
}

void InfrastructureImpl::reset_all_rewards() {
    for (auto [p, r] : simulated_policies()) {
        r->reset(p->peek());
    }
    auto [p, r] = active_policy();
    r->reset(p->peek());
}

void InfrastructureImpl::tick() {
    since_last_batch_++;
    if (since_last_batch_ == params_.batch_size()) {
        finish_batch();
        start_batch();
    }
}

void InfrastructureImpl::finish_batch() {
    vector<optional<Reward>> rewards(policies_.size(), std::nullopt);
    rewards[active_policy_idx_] = rewards_pool_.front()->get();
    for (auto i = 0u; i < simulated_policies_idxs_.size(); ++i) {
        rewards[simulated_policies_idxs_[i]] = rewards_pool_[i + 1]->get();
    }

    learning_->report_rewards(rewards);
    if (stat_collector_) {
        stat_collector_->append_step(
            active_policy_idx_, rewards_pool_.front()->get()
        );
    }
    simulated_policies_idxs_.clear();
}

void InfrastructureImpl::start_batch() {
    auto const chosen = learning_->choose();
    change_current(chosen.front());

    std::copy(
        begin(chosen) + 1, 
        begin(chosen) + (std::min(1 + params_.num_simulated(), chosen.size())),
        back_inserter(simulated_policies_idxs_)
    );

    since_last_batch_ = 0;
    reset_all_rewards();
}

void InfrastructureImpl::change_current(size_t active_policy_idx) {
    if (active_policy_idx_ != active_policy_idx) {
        auto real_packets = policies_[active_policy_idx_]->take();
        auto simulated_packets = policies_[active_policy_idx]->take();
        policies_[active_policy_idx_]->admit_n_drop(std::move(simulated_packets));

        active_policy_idx_ = active_policy_idx;

        auto dropped = policies_[active_policy_idx]->admit_n_drop(std::move(real_packets));
        assert(dropped.empty());
    }
}

auto InfrastructureImpl::simulated_policies() const
        -> vector<tuple<PolicyInstance *, RewardFunction *>> {
    vector<tuple<PolicyInstance *, RewardFunction*>> result(simulated_policies_idxs_.size());
    for (auto i = 0u; i < result.size(); ++i) {
        result[i] = make_tuple(
            policies_[simulated_policies_idxs_[i]].get(), rewards_pool_[i + 1].get()
        );
    }
    return result;
}

auto InfrastructureImpl::active_policy() const 
        -> tuple<PolicyInstance *, RewardFunction *> {
    return std::make_tuple(
        policies_[active_policy_idx_].get(), rewards_pool_.front().get()
    );
}
}
