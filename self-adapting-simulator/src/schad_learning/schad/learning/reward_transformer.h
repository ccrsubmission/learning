#ifndef REWARD_TRANSFORMER_H
#define REWARD_TRANSFORMER_H

#include <schad/learning/learning_method_factory_helper.h>

namespace schad::learning {

template<class Func> 
class RewardTransformerLearningMethod : public LearningMethod {
public:
    RewardTransformerLearningMethod(Func func, unique_ptr<LearningMethod> base)
        : func_{func}, base_{std::move(base)} {
    }

    void report_rewards(vector<optional<Reward>> const& rewards) override {
        vector<optional<Reward>> normalized_rewards(rewards.size());
        std::transform(begin(rewards), end(rewards), begin(normalized_rewards), 
            [this] (auto const& r) -> optional<Reward> {
                if (r.has_value()) {
                    return func_(*r);
                } else {
                    return std::nullopt;
                }
            }
        );
        base_->report_rewards(normalized_rewards);
    }

    auto choose() -> vector<size_t> override {
        return base_->choose();
    }

private:
    Func const func_;
    unique_ptr<LearningMethod> const base_;
};

template<class Func>
auto create_reward_tranformer(
        std::shared_ptr<LearningMethodFactory> base, Func func, json const& j) {
    return create_learning_factory(
        [base, func] (auto rng, auto num_arms) {
            return make_unique<RewardTransformerLearningMethod<Func>>(
                func, base->instantiate(rng, num_arms)
            );
        }, j);
}

}

#endif // REWARD_TRANSFORMER_H
