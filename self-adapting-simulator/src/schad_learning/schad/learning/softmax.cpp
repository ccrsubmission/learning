#include "softmax.h"
#include <schad/learning/learning_method_factory_helper.h>

namespace {

using namespace schad;
using namespace learning;

class SoftMaxLearningMethod : public LearningMethod {
public:
    SoftMaxLearningMethod(double alpha, size_t num_arms, shared_ptr<rng_t> rng)
        : rng_{std::move(rng)}, alpha_{alpha}, 
          average_{0.0}, count_{0}, preferences_(num_arms) {
    }


    auto choose() -> vector<size_t> override {
        auto const ps = get_probabilities();
        std::discrete_distribution<size_t> d(begin(ps), end(ps));
        return {d(*rng_)};
    }

    void report_rewards(vector<optional<Reward>> const& rewards) override {
        auto const ps = get_probabilities();
        auto total_known_prob = 0.0;
        auto total_known_reward = 0.0; 
        for (auto i = 0u; i < ps.size(); i++) {
            if (rewards[i].has_value()) {
                total_known_prob += ps[i];
                total_known_reward += rewards[i]->value() * ps[i];
            }
        }

        for (auto i = 0u; i < ps.size(); i++) {
            if (rewards[i].has_value()) {
                preferences_[i] += alpha_ * (rewards[i]->value() - average_) 
                    * (ps[i] / total_known_prob - ps[i]);
            } else {
                preferences_[i] -= alpha_ * 
                    (total_known_reward / total_known_prob - average_) * ps[i];
            }
        }
        for (auto i = 0u; i < ps.size(); i++) {
            if (rewards[i].has_value()) {
                average_ += (rewards[i]->value() - average_) / double(1 + count_);
                count_++;
            }
        }
    }

private:
    auto get_probabilities() const -> vector<double> {
        double m = *std::min_element(begin(preferences_), end(preferences_));
        vector<double> ps(preferences_.size());
        std::transform(begin(preferences_), end(preferences_),
                       begin(ps), [m] (auto x) { return std::exp(x - m); });
        auto const total = std::accumulate(begin(ps), end(ps), 0.0);
        for (auto& x : ps) {
            x /= total;
        }

        return ps;
    }

private:
    shared_ptr<rng_t> const rng_;
    double const alpha_;

    double average_;
    size_t count_;
    vector<double> preferences_;
};

}

auto schad::learning::create_softmax(double alpha) 
    -> unique_ptr<LearningMethodFactory> {
    return create_learning_factory(
        [alpha] (auto rng, auto num_arms) {
            return make_unique<SoftMaxLearningMethod>(alpha, num_arms, rng);
        },
        {{"type", "softmax"},
         {"parameters", {{"alpha", alpha }}}}
    );
}
