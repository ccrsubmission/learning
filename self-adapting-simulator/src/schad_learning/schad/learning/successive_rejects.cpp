#include "successive_rejects.h"

#include <schad/learning/average_func.h>

namespace {

using namespace schad;
using namespace learning;

class SuccessiveRejectsArmIdentificationMethod
        : public ArmIdentificationMethod {
private:
    using Avg = AverageFunc<KeepAll>;
public:
    SuccessiveRejectsArmIdentificationMethod(size_t num_arms, size_t time_limit)
        : num_arms_{num_arms}
        , time_limit_{time_limit} 
        , phase_rounds_remain_{0}
        , phase_round_arms_remain_{0}
        , remaining_arms_(num_arms)
        , arm_rewards_(num_arms, Avg{KeepAll{}})
        {
        iota(begin(remaining_arms_), end(remaining_arms_), 0);
        reset_round();
    }

    void report_rewards(vector<optional<Reward>> const& rewards) override {
        for (auto i = 0u; i < num_arms_; i++) {
            arm_rewards_[i] += maybe_value(rewards[i]);
        }
        next_arm() || next_round() || next_phase();
    }

    auto choose() -> vector<size_t> override {
        return remaining_arms_;
    }

    auto get_best() const -> optional<size_t> override {
        if (remaining_arms_.size() == 1) {
            return remaining_arms_.front();
        } else {
            return std::nullopt;
        }
    }

private:
    static auto get_logk_bound(size_t num_arms) -> double {
        double result = 0.5;
        for (auto i = 2u; i <= num_arms; i++) {
            result += 1 / (double) i;
        }
        return result;
    }

    auto next_arm() -> bool {
        if (phase_round_arms_remain_ > 0) {
            phase_round_arms_remain_--;
        }
        if (phase_round_arms_remain_ == 0) {
            return false;
        }
        rotate(begin(remaining_arms_), 
               begin(remaining_arms_) + 1, 
               end(remaining_arms_)); 

        return true;
    }

    void reset_arm() {
        phase_round_arms_remain_ = remaining_arms_.size();
    }

    auto next_round() -> bool {
        if (phase_rounds_remain_ > 0) {
            phase_rounds_remain_--;
        }
        if (phase_rounds_remain_ == 0) {
            return false;
        }
        reset_arm();
        return true;
    }

    void reset_round() {
        phase_rounds_remain_ = get_num_rounds();
        reset_arm();
    }

    auto next_phase() -> bool {
        if (remaining_arms_.size() == 1) {
            return false;
        }
        auto the_worst = min_element(
                begin(remaining_arms_), end(remaining_arms_), [this]
                (auto i, auto j) { 
                    return arm_rewards_[i].avg() < arm_rewards_[j].avg();
                });
        remaining_arms_.erase(the_worst);
        reset_round();
        return true;
    }

    auto get_num_rounds() const -> size_t {
        return get_nk(get_phase_idx()) - get_nk(get_phase_idx() - 1);
    }

    auto get_phase_idx() const -> size_t {
        return num_arms_ - remaining_arms_.size() + 1;
    }

    auto get_nk(size_t k) const -> size_t {
        if (k == 0) {
            return 0;
        }
        return ceil(
            (time_limit_ - num_arms_) 
            / (double) (get_logk_bound(num_arms_) * (num_arms_ + 1 - k))
        );
    }

private:
    size_t const num_arms_;
    size_t const time_limit_;

    size_t phase_rounds_remain_;
    size_t phase_round_arms_remain_;
    vector<size_t> remaining_arms_;
    vector<Avg>       arm_rewards_;
};

class SuccessiveRejectsArmIdentificationMethodFactory 
        : public ArmIdentificationMethodFactory {
public:
    SuccessiveRejectsArmIdentificationMethodFactory() = default;


    auto instantiate([[maybe_unused]] shared_ptr<rng_t> rng,
                                      size_t       num_arms, 
                                      size_t     time_limit) const
        -> unique_ptr<ArmIdentificationMethod> override {
        return make_unique<SuccessiveRejectsArmIdentificationMethod>(
                num_arms, time_limit
        );
    }


    void to_json(json& j) const override {
        j = {{"type", "successive_rejects"},
             {"parameters", {}}};
    }

};
}

auto schad::learning::create_successive_rejects() 
    -> unique_ptr<ArmIdentificationMethodFactory> {
    return make_unique<SuccessiveRejectsArmIdentificationMethodFactory>();
}
