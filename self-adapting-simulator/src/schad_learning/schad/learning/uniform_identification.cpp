#include "uniform_identification.h"

#include <schad/learning/average_func.h>

namespace {

using namespace schad;
using namespace learning;

class UniformArmIdentificationMethod : public ArmIdentificationMethod {
private:
    using Avg = AverageFunc<KeepAll>;
public:
    UniformArmIdentificationMethod(size_t num_arms, size_t time_limit) 
        : avg_reward_(num_arms, Avg{KeepAll{}})
        , arms_(num_arms)
        , time_limit_{time_limit}
        , current_time_{0} {
        iota(begin(arms_), end(arms_), 0);
    }

    void report_rewards(vector<optional<Reward>> const& rewards) override {
        for (auto i = 0u; i < rewards.size(); i++) {
            avg_reward_[i] += maybe_value(rewards[i]);
        }
        current_time_++;
    }

    auto choose() -> vector<size_t> override { 
        auto result = arms_;
        rotate(begin(arms_), begin(arms_) + 1, end(arms_));
        return move(result);
    }

    auto get_best() const -> optional<size_t> override {
        if (current_time_ > time_limit_) {
            return max_element(begin(avg_reward_), end(avg_reward_),
                    [] (auto const& a, auto const& b) { 
                        return a.avg() < b.avg();
                        }) - begin(avg_reward_);
        } else {
            return std::nullopt; 
        }
    }

private:
    vector<Avg> avg_reward_;
    vector<size_t> arms_;

    size_t const time_limit_;
    size_t current_time_;
};

class UniformArmIdentificationMethodFactory 
    : public ArmIdentificationMethodFactory {
    public:

    UniformArmIdentificationMethodFactory() = default;

    auto instantiate([[maybe_unused]]shared_ptr<rng_t> rng,
                                     size_t       num_arms, 
                                     size_t     time_limit) const
            -> unique_ptr<ArmIdentificationMethod> override {
        return make_unique<UniformArmIdentificationMethod>(num_arms, time_limit);
    }


    void to_json(json& j) const override {
        j = {{"type", "uniform"}, 
             {"parameters", {}}};
    }
};

} // namespace

auto schad::learning::create_uniform() 
    -> unique_ptr<ArmIdentificationMethodFactory> {
    return make_unique<UniformArmIdentificationMethodFactory>();
}
