#ifndef REWARD_FUNCTION_H
#define REWARD_FUNCTION_H

#include <schad/reward/reward.h>

namespace schad {

struct RewardFunction {
    virtual void note_transmission(uint32_t time, Packet const& p) = 0;
    virtual void note_arrival(Packet const& p) = 0;

    virtual auto get() const -> Reward = 0;
    virtual void reset(vector<Packet const*> const& buffer) = 0;

    virtual auto clone() const -> unique_ptr<RewardFunction> = 0;
    virtual void to_json(json& j) const = 0;

    RewardFunction() = default;
    RewardFunction(RewardFunction const&) = delete;
    RewardFunction& operator=(RewardFunction const&) = delete;

    virtual ~RewardFunction() = default;
};

inline void to_json(json& j, RewardFunction const& r) {
    r.to_json(j);
}

}

#endif
