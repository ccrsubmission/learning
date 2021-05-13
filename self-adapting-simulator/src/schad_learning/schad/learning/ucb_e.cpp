#include "ucb_e.h"
#include <schad/learning/ucb_common.h>
#include <schad/learning/learning_method_factory_helper.h>
#include <schad/learning/average_func.h>

namespace {
using namespace schad;
using namespace learning;

class UCBExploration : 
    public UCBCommon<UCBExploration, KeepAll> {
public:
    UCBExploration(double a, size_t num_arms) 
        : UCBCommon<std::decay_t<decltype(*this)>, KeepAll>{num_arms, KeepAll{}}
        , a_{a} {
    }

    auto get_prio() {
        return [this] (auto i) {
            return this->value(i).avg() + sqrt(a_ / this->count(i));
        };
    }

private:
    double const a_;
};

}

auto schad::learning::create_ucb_e(double a) 
        -> unique_ptr<LearningMethodFactory> {
    return create_learning_factory(
        [a] ([[maybe_unused]] auto rng, auto num_arms) {
            return make_unique<UCBExploration>(a, num_arms);
        },
        {{"type", "ucb_e"},{"parameters", {{"a", a}}}}
    );
}
