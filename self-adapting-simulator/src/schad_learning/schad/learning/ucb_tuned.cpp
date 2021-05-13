#include "ucb_tuned.h"
#include <schad/learning/ucb_common.h>
#include <schad/learning/average_func.h>
#include <schad/learning/learning_method_factory_helper.h>

namespace {

using namespace schad;
using namespace learning;

template<class Average>
class UCBTunedLearningMethod : public UCBCommon<UCBTunedLearningMethod<Average>, Average> {
    using AvgSq = AverageFunc<Average, true>;
public:
    static constexpr auto const need_reward = true;

    UCBTunedLearningMethod(size_t num_arms, Average avg)
        : UCBCommon<UCBTunedLearningMethod<Average>, Average>(num_arms, avg),
          squares_(num_arms, AvgSq{avg}) {
    }

    void report_reward(optional<double> const& reward, size_t arm) {
        squares_[arm] += reward;
    }

    auto get_prio() {
        return [total_count=this->total_count(),this] (auto i) {
            return this->value(i).avg() + sqrt(log(total_count) / this->count(i) * 
                std::min(0.25, squares_[i].avg() - this->value(i).avg() * this->value(i).avg() 
                    + sqrt(2.0 * log(total_count) / this->count(i))
                )
            );
        };
    }

private:
    vector<AvgSq> squares_;
};

}

auto schad::learning::create_ucb_tuned(Average avg) 
    -> unique_ptr<LearningMethodFactory> {
    return std::visit([](auto&& avg) {
        return create_learning_factory(
            [avg]([[maybe_unused]] auto rng, auto num_arms) {
                using T = std::decay_t<decltype(UCBTunedLearningMethod(num_arms, avg))>;
                return make_unique<T>(num_arms, avg);
            },
            {{"type", "ucb_tuned"},{"parameters", {{"average", avg}}}}
        );
    }, avg);
}
