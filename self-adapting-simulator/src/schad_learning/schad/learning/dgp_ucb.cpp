#include "dgp_ucb.h"
#include <schad/learning/learning_method_factory_helper.h>
#include <schad/learning/average_func.h>


namespace {

using namespace schad;
using namespace learning;

template<bool ucb_counts=false, class Average=KeepAll>
class DGPUCBLearningMethod : public LearningMethod {
private:
    using Avg = AverageFunc<Average>;

public:
    DGPUCBLearningMethod(double delta, double ksi, Average avg, size_t num_arms)
        : delta_{delta}, ksi_{ksi}, init_arm_{0}, 
         us_(num_arms), total_(num_arms), ucb_counts_(num_arms, Avg{avg}) {
    }

    auto choose() -> vector<size_t> override {
        if (init_arm_ < us_.size()) {
            return {init_arm_++};
        }

        
        auto const get_count = [this](auto i) {
            if constexpr(ucb_counts) {
                return ucb_counts_[i].count();
            } else {
                return total_[i];
            }
        };

        auto const total_count = [this,get_count]() {
            auto count = 0.0;
            for (auto i = 0u; i < us_.size(); i++) {
                count += get_count(i);
            }
            return count;
        }();


        auto get_prio = [this,total_count,get_count] (auto i) {
            return us_[i] / (total_[i] + 0.0001) 
                + sqrt(ksi_ * log(total_count) / get_count(i));
        };

        vector<size_t> result(us_.size());
        std::iota(begin(result), end(result), 0);

        sort(begin(result), end(result), [get_prio](auto a, auto b) {
            return get_prio(a) > get_prio(b);
        });

        return result;
    }

    void report_rewards(vector<optional<Reward>> const& rewards) override {
        for (auto i = 0u; i < rewards.size(); i++) {
            total_[i] *= delta_;
            us_[i] *= delta_;

            if (rewards[i].has_value()) {
                us_[i] += rewards[i]->value();
                total_[i] += *rewards[i]->total();
            }

            if constexpr(ucb_counts) {
                ucb_counts_[i] += maybe_value(rewards[i]);
            }
        }
    }

private:
    double const delta_;
    double const ksi_;
    size_t init_arm_;
    vector<double> us_;
    vector<double> total_;
    vector<Avg> ucb_counts_;
};

}

auto schad::learning::create_dgp_ucb(double delta, double ksi, optional<Average> avg) 
    -> unique_ptr<LearningMethodFactory> {
    return create_learning_factory(
        [delta,ksi,avg] ([[maybe_unused]] auto rng, auto num_arms) {
            return std::visit([delta,ksi,num_arms,ucb_counts=avg.has_value()] 
                (auto const& avg) -> unique_ptr<LearningMethod> {
                    using T = std::decay_t<decltype(avg)>;
                    if (ucb_counts) {
                        return make_unique<DGPUCBLearningMethod<true, T>>(delta, ksi, avg, num_arms);
                    } else {
                        return make_unique<DGPUCBLearningMethod<>>(delta, ksi, KeepAll{}, num_arms);
                    }
                },
                avg.value_or(KeepAll{})
            );
        }, 
        {{"type", "dgp_ucb"}, {"parameters", {
            {"delta", delta},
            {"ksi", ksi},
            {"average",  avg.has_value() ? json(*avg) : json(nullptr)}
        }}}
    );
}
