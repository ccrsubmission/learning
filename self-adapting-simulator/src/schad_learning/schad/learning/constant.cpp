#include "constant.h"
#include <schad/learning/learning_method_factory_helper.h>

#include <exception>

namespace {
using namespace schad;
using namespace learning;

class ConstantLearningMethod : public LearningMethod {
public:
    ConstantLearningMethod(size_t arm_idx) : arm_idx_{arm_idx} {
    }

    void report_rewards([[maybe_unused]] vector<optional<Reward>> const& rewards) override {
        
    }

    auto choose() -> vector<size_t> override {
        return {arm_idx_};
    }
private:
    size_t const arm_idx_;
};

}

auto schad::learning::create_constant(size_t arm_idx) 
        -> unique_ptr<LearningMethodFactory> {
    return create_learning_factory([arm_idx]
        ([[maybe_unused]] auto rng, auto num_arms) {
            if (num_arms <= arm_idx) {
                throw std::invalid_argument(
                    "There must be at least " 
                        + std::to_string(arm_idx + 1) + "arms!");
            }
            return make_unique<ConstantLearningMethod>(arm_idx);
        },
        {{"type", "constant"}, {"parameters", {{"arm_idx", arm_idx}}}}
    );
}
