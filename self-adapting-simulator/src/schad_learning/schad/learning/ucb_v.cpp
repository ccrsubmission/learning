#include "ucb_v.h"

#include <schad/learning/ucb_common.h>
#include <schad/learning/learning_method_factory_helper.h>

namespace {

using namespace schad;
using namespace learning;

template<class Average>
class UCBVLearningMethod : public UCBCommon<UCBVLearningMethod<Average>, Average> {
    using AvgSq = AverageFunc<Average, true>;
public:
    static constexpr auto const need_reward = true; 

    UCBVLearningMethod(size_t num_arms, Average avg) 
        : UCBCommon<std::decay_t<decltype(*this)>, Average>(num_arms, avg)
        , squares_(num_arms, AvgSq{avg}) {
    }

    void report_reward(optional<double> const& reward, size_t arm) {
        squares_[arm] += reward;
    }

    auto get_prio() {
        return [total_count=this->total_count(),this] (auto i) {
            return this->value(i).avg() + sqrt(2 * log(total_count) / this->count(i) *
                    (squares_[i].avg() - this->value(i).avg() * this->value(i).avg())
                    ) + log(total_count) / this->count(i);
                     
        };
    }
private:
    vector<AvgSq> squares_;
};

}

auto schad::learning::create_ucb_v(Average avg) 
    -> unique_ptr<LearningMethodFactory> {
    return std::visit([](auto&& avg) {
        return create_learning_factory(
            [avg] ([[maybe_unused]] auto rng, auto num_arms) {
                using T = std::decay_t<decltype(UCBVLearningMethod(num_arms, avg))>;
                return make_unique<T>(num_arms, avg);
            },
            {{"type", "ucb_v"}, {"parameters", {{"average", avg}}}}
        );
    }, avg);
}
