#ifndef NS_INTERVAL_SELECTOR_H
#define NS_INTERVAL_SELECTOR_H

#include "object.h"
#include "interval_params.h"
#include "reward_listener.h"

#include <vector>
#include <optional>

struct IntervalSelector : TclObject, RewardListener {
    IntervalSelector() = default;
    IntervalSelector(IntervalSelector const&) = delete;
    IntervalSelector& operator=(IntervalSelector const&) = delete;

    virtual auto take_new_params() -> optional<IntervalParams> = 0;

    virtual void reset(size_t num_arms) = 0;

    virtual ~IntervalSelector() = default;
};

#endif // NS_INTERVAL_SELECTOR_H
