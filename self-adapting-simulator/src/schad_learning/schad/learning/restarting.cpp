#include "restarting.h"
#include <schad/learning/learning_method_factory_helper.h>

namespace {

using namespace schad;
using namespace learning;

template<class Restarter>
class RestartingLearningMethod : public LearningMethod {
public:
    RestartingLearningMethod(Restarter restarter, size_t num_steps)
        : base_{}, restarter_{restarter}, 
          num_steps_{num_steps}, num_steps_phase_{0} {
        restart();
    }


    void report_rewards(vector<optional<Reward>> const& rewards) override {
        base_->report_rewards(rewards);
    }

    auto choose() -> vector<size_t> override {
        num_steps_phase_++;
        if (num_steps_phase_ == num_steps_) {
            restart();
        }
        return base_->choose();
    }
private:
    void restart() {
        base_ = restarter_();
        num_steps_phase_ = 0;
    }

private:
    unique_ptr<LearningMethod> base_;
    Restarter const restarter_;
    size_t const num_steps_;
    size_t num_steps_phase_;
};

}

auto schad::learning::create_restarting(
        shared_ptr<LearningMethodFactory> base, size_t num_steps)
    -> unique_ptr<LearningMethodFactory> {
    return create_learning_factory([base=base, num_steps]
        (auto rng, auto num_arms) {
            auto const restarter = [base, num_arms, rng] { 
                return base->instantiate(rng, num_arms); 
            };
            return make_unique<RestartingLearningMethod<decltype(restarter)>>(
                restarter, num_steps
            );
        },
        {
            {"type", "restarting"},
            {"parameters", {
                {"base", *base},
                {"num_steps", num_steps}
            }}
        }
    );
}
