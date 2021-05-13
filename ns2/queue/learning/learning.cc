#include "learning.h"
#include "learning_impl.h"

auto Learning::build(
        vector<Queue *> policies,
        shared_ptr<schad::learning::LearningMethodFactory> learning,
        Reward const& reward
        ) -> unique_ptr<Learning> {
    return make_unique<LearningImpl>(move(policies), move(learning), reward);
}
