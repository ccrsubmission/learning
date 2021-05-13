#include "combined.h"
#include <schad/learning/learning_method_factory_helper.h>

namespace {

using namespace schad;
using namespace learning;


class CombinedLearningMethod : public LearningMethod {
public:
    CombinedLearningMethod(std::unique_ptr<LearningMethod> exploiter,
                           std::unique_ptr<LearningMethod> explorer) 
        : exploiter_{std::move(exploiter)}, explorer_{std::move(explorer)} {
    }

    void report_rewards(vector<optional<Reward>> const& rewards) override {
        exploiter_->report_rewards(rewards);
        explorer_->report_rewards(rewards);
    }

    auto choose() -> vector<size_t> override {
        auto const exploited = exploiter_->choose().front();
        auto result = explorer_->choose();
        auto expl_it = std::find(begin(result), end(result), exploited);

        if (expl_it == end(result)) {
            result.emplace_back(exploited);
            expl_it = end(result) - 1;
        }

        std::swap(result.front(), *expl_it);
        return result;
    }

private:
    unique_ptr<LearningMethod> const exploiter_;
    unique_ptr<LearningMethod> const explorer_;
};

}

auto schad::learning::create_combined(
        shared_ptr<LearningMethodFactory> exploiter,
        shared_ptr<LearningMethodFactory> explorer) 
        -> unique_ptr<LearningMethodFactory> {
    return create_learning_factory(
        [exploiter, explorer]
        (auto rng, auto num_arms) {
            return make_unique<CombinedLearningMethod>(
                exploiter->instantiate(rng, num_arms), 
                explorer->instantiate(rng, num_arms)
            );
        },
        {
            {"type", "combined"},
            {"parameters", {
                {"exploiter", *exploiter},
                {"explorer", *explorer}
            }}
        }
    );
}
