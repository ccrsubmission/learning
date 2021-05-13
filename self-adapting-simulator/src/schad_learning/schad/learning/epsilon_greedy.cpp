#include "epsilon_greedy.h"
#include <schad/learning/learning_method_factory_helper.h>
#include <schad/learning/average_func.h>

namespace {
using namespace schad;
using namespace learning;

template<class Average>
class EpsilonGreedyLearningMethod : public LearningMethod {
private:
    using Avg = AverageFunc<Average>;
public:
    EpsilonGreedyLearningMethod(
            shared_ptr<rng_t> rng, double epsilon, double delta, double sigma,
            size_t num_arms, Average avg
        ) : init_arm_{0},
            rng_{std::move(rng)}, epsilon_{epsilon}, delta_{delta}, sigma_{sigma},
            rnd_idx_{0, num_arms - 1}, arm_values_(num_arms, Avg{avg}) {
    }

    void report_rewards(vector<optional<Reward>> const& rewards) override {
        auto total_change = 0.0;
        auto num_changes = 0;
        for (auto i = 0u; i < rewards.size(); i++) {
            if (rewards[i].has_value()) {
                auto const before = arm_values_[i].avg();
                arm_values_[i] += rewards[i]->value();
                total_change += arm_values_[i].avg() - before;
                num_changes++;
            }
        }
        epsilon_ += delta_ * (
            (1 - exp(-abs(total_change / num_changes) / sigma_))
            /
            (1 + exp(-abs(total_change / num_changes) / sigma_))
            - epsilon_
        );
    }

    auto choose() -> vector<size_t> override {
        if (init_arm_ < arm_values_.size()) {
            return {init_arm_++};
        }
        std::bernoulli_distribution coin{epsilon_};
        vector<size_t> result(arm_values_.size());
        std::iota(begin(result), end(result), 0);

        if (coin(*rng_)) {
            std::shuffle(begin(result), end(result), *rng_);
        } else {
            std::sort(begin(result), end(result), [this] (auto i, auto j) {
                return arm_values_[i].avg() > arm_values_[j].avg();
            });
        }
        return result;
    }

private:
    size_t init_arm_;
    shared_ptr<rng_t> const rng_;
    double epsilon_;
    double const delta_;
    double const sigma_;
    std::uniform_int_distribution<size_t> rnd_idx_;
    vector<Avg> arm_values_;
};

}

auto schad::learning::create_epsilon_greedy(EpsilonGreedyParameters const& params) 
    -> unique_ptr<LearningMethodFactory> {
    return create_learning_factory([params=params]
        (auto rng, auto num_arms) {
            return std::visit([rng,num_arms,&params] (auto&& avg) 
                    -> unique_ptr<LearningMethod> {
                using T = std::decay_t<decltype(avg)>;
                return make_unique<EpsilonGreedyLearningMethod<T>>(
                        std::move(rng), params.epsilon(), params.delta(), 
                        params.sigma(), num_arms, avg
                );
            }, params.average());
        },
        {   
            {"type", "epsilon_greedy"},
            {"parameters", params}
        }
    );
}
