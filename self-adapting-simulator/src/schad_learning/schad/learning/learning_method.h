#ifndef LEARNING_METHOD_H
#define LEARNING_METHOD_H

#include <schad/reward/reward.h>
#include <schad/common.h>

namespace schad::learning {
struct LearningMethod {
    virtual void report_rewards(vector<optional<Reward>> const& rewards) = 0;
    virtual auto choose() -> vector<size_t> = 0;

    LearningMethod() = default; 

    LearningMethod(LearningMethod const&) = delete;
    LearningMethod& operator=(LearningMethod const&) = delete;

    virtual ~LearningMethod() = default;
};

struct LearningMethodFactory {
    virtual auto instantiate(shared_ptr<rng_t> rng, size_t num_arms) const 
        -> unique_ptr<LearningMethod> = 0;

    virtual void to_json(json& j) const = 0;

    LearningMethodFactory() = default;
    LearningMethodFactory(LearningMethodFactory const&) = delete;
    LearningMethodFactory& operator=(LearningMethodFactory const&) = delete;

    virtual ~LearningMethodFactory() = default;
};

inline void to_json(json& j, LearningMethodFactory const& learning) {
    learning.to_json(j);
}

}

#endif // LEARNING_METHOD_H
