#ifndef UCB_COMMON_H
#define UCB_COMMON_H

#include <schad/learning/learning_method.h>
#include <schad/learning/average_func.h>

namespace schad::learning {

template <class Derived, class Average>
struct UCBCommon : public LearningMethod {
    static constexpr auto const need_choice = false;
    static constexpr auto const provides_special_count = false;
    static constexpr auto const need_reward = false;

    using Avg = AverageFunc<Average>;

    UCBCommon(size_t num_arms, Average avg) : 
        num_arms_{num_arms}, init_arm_{0}, values_(num_arms, Avg{avg}) {
    }

    void report_rewards(vector<optional<Reward>> const& rewards) override {
        for (auto i = 0; i < int(rewards.size()); i++) {
            if constexpr(Derived::need_reward) {
                static_cast<Derived *>(this)->report_reward(maybe_value(rewards[i]), i);
            }
            values_[i] += maybe_value(maybe_value(rewards[i]));
        }
    }

    auto choose() -> vector<size_t> override {
        if (init_arm_ < num_arms_) {
            if constexpr(Derived::need_choice) {
                static_cast<Derived *>(this)->report_choice({init_arm_});
            }
            return {init_arm_++};
        }
        auto const get_prio = static_cast<Derived *>(this)->get_prio();
        vector<size_t> result(num_arms_);
        std::iota(begin(result), end(result), 0);

        sort(begin(result), end(result), [get_prio](auto a, auto b) {
            return get_prio(a) > get_prio(b);
        });

        if constexpr(Derived::need_choice) {
            static_cast<Derived *>(this)->report_choice(result);
        }

        return result;
    }

    auto total_count() const {
        double total = 0;
        for (int i = 0; i < int(num_arms_); i++) {
            total += count(i);
        }
        return total;
    }

    auto count(size_t i) const {
        if constexpr(Derived::provides_special_count) {
            return static_cast<Derived const *>(this)->special_count(i);
        } else {
            return values_[i].count();
        }
    }

    auto const& value(size_t arm) const {
        return values_[arm];
    }

private:
    size_t const num_arms_;
    size_t init_arm_;
    vector<Avg> values_;
};

}

#endif // USB_COMMON_H
