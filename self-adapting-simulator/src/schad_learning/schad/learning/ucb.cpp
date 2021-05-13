#include "ucb.h"
#include <schad/learning/ucb_common.h>
#include <schad/learning/average_func.h>
#include <schad/learning/learning_method_factory_helper.h>

namespace {
using namespace schad;
using namespace learning;

template<class Average, bool restricted_exploration>
class UCBLearningMethod 
    : public UCBCommon<UCBLearningMethod<Average, restricted_exploration>, Average> {
private:

public:
    using Avg = AverageFunc<Average>;
    static constexpr auto const need_choice = restricted_exploration;
    static constexpr auto const provides_special_count = restricted_exploration;

    UCBLearningMethod(double ksi, Average avg, size_t num_arms)
        : UCBCommon<std::decay_t<decltype(*this)>, Average>(num_arms, avg),
          ksi_{ksi}, last_choice_{0}, my_value_(num_arms, Avg{avg}) {
    }

    auto special_count(size_t i) const {
        if constexpr(restricted_exploration) {
            return my_value_[i].count();
        } else {
            exit(1);
        }
    }

    auto get_prio() {
        return [this,total_count=this->total_count()] (auto i) {
                return this->value(i).avg() 
                    + sqrt(ksi_ * log(total_count) / this->count(i));
        };
    }

    auto report_choice(vector<size_t> const& rewards) {
        if constexpr (restricted_exploration) {
            last_choice_ = rewards.front();
        }
    }

    void report_reward(optional<double> const& r, size_t arm) {
        if constexpr(restricted_exploration) {
            if (arm == last_choice_) {
                my_value_[arm] += r;
            }
        }
    }

private:
    double const ksi_;

    size_t last_choice_;
    vector<Avg> my_value_;
};

}

auto schad::learning::create_ucb(UCBParameters const& params) 
        -> unique_ptr<LearningMethodFactory> {
    return create_learning_factory([params=params]
        (auto rng, auto num_arms) {
            return std::visit([rng,num_arms,&params] (auto&& avg) -> unique_ptr<LearningMethod> {
                using T = std::decay_t<decltype(avg)>;
                if (params.restricted_exploration()) {
                    return make_unique<UCBLearningMethod<T,true>>(params.ksi(), avg, num_arms);
                } else {
                    return make_unique<UCBLearningMethod<T,false>>(params.ksi(), avg, num_arms);
                }
            }, params.average());
        },
        {{"type", "ucb"}, {"parameters", params}}
    );
}
