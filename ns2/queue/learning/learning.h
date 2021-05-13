#ifndef NS_LEARNING_H
#define NS_LEARNING_H

#include "learning_common.h"
#include "queue.h"
#include "reward.h"
#include "interval_params.h"
#include "reward_listener.h"

#include <memory>
#include <schad/learning/learning_method.h>

struct IntervalEndListener {
    virtual void interval_ended() = 0;

    IntervalEndListener() = default;
    IntervalEndListener(IntervalEndListener const&) = delete;
    IntervalEndListener& operator=(IntervalEndListener const&) = delete;

    virtual ~IntervalEndListener() = default;
};

struct Learning {
    static auto build(
        vector<Queue *> policies, 
        shared_ptr<schad::learning::LearningMethodFactory> learning,
        Reward const& reward
    ) -> unique_ptr<Learning>;

    virtual void set_interval_end_listener(IntervalEndListener * listener) = 0;
    virtual void set_reward_listener(RewardListener * listener) = 0;

    virtual void restart(IntervalParams const& params) = 0;

    virtual void note_arrival(Packet const * pkt) = 0;
    virtual void note_drop(Packet const * pkt) = 0;
    virtual void note_transmission(Packet const * pkt) = 0;

    virtual auto get_current() const -> Queue * = 0;

    virtual void write_stats(ostream& out) const = 0;
    virtual void write_reward_stats(ostream& out) const = 0;

    Learning() = default;
    Learning(Learning const&) = delete;
    Learning& operator=(Learning const&) = delete;

    virtual ~Learning() = default;
};

#endif // NS_LEARNING_H
