#include "scaled.h"
#include <schad/learning/reward_transformer.h>

auto schad::learning::create_scaled(shared_ptr<LearningMethodFactory> base, 
        double factor) -> unique_ptr<LearningMethodFactory> {
    return create_reward_tranformer(base,
        [factor] (auto const& r) {
            auto const value = r.value() * factor;
            if (r.total().has_value()) {
                return Reward{value, *r.total() * factor};
            } else {
                return Reward{value, std::nullopt};
            }
        },
        {{"type", "scaled"},
         {"parameters", {{"factor", factor}, {"base", *base}}}}
    );
}
