#include "normalized.h"
#include <schad/learning/reward_transformer.h>

auto schad::learning::create_normalized(
        std::shared_ptr<LearningMethodFactory> base) 
    -> unique_ptr<LearningMethodFactory> {
    return create_reward_tranformer(base,
        [] (auto const r) -> optional<Reward> { 
            if (r.total().has_value() && *r.total() != 0) {
                return Reward{r.value() / *r.total(), std::nullopt};
            } else {
                return std::nullopt;
            }
        },
        {{"type", "normalized"},{"parameters", {{"base", *base}}}}
    );
}
