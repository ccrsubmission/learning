#include "local_greedy.h"
#include <schad/learning/learning_method_factory_helper.h>

namespace {

using namespace schad;
using namespace learning;

class LocalGreedyLearningMethod : public LearningMethod {
public:
    explicit LocalGreedyLearningMethod(size_t num_arms)
        : init_arm_{0}, rewards_(num_arms) {
    }

    void report_rewards(vector<optional<Reward>> const& rewards) override {
        if (init_arm_ < rewards_.size()) {
            rewards_[init_arm_] = 
                rewards[init_arm_]->value();
            init_arm_++;
        }
    }

    auto choose() -> vector<size_t> override {
        if (init_arm_ < 1 * rewards_.size()) {
            return {init_arm_};
        } else {
            return {(size_t)(
                max_element(begin(rewards_), end(rewards_)) - begin(rewards_)
            )};
        }
    }

private:
    size_t init_arm_;
    vector<double> rewards_;
};

}

auto schad::learning::create_local_greedy() 
        -> unique_ptr<LearningMethodFactory> {
    return create_learning_factory(
        [] (auto rng, auto num_arms) {
            return make_unique<LocalGreedyLearningMethod>(num_arms);
        },
        {{"type", "local_greedy"}, {"parameters", {}}}
    );
}
