#ifndef NS_LEARNING_IMPL_H
#define NS_LEARNING_IMPL_H

#include "learning.h"

class LearningImpl : public Learning {
public:
    LearningImpl(
        vector<Queue *> policies, 
        shared_ptr<schad::learning::LearningMethodFactory> learning,
        Reward const& reward
    );
    ~LearningImpl();

    void set_interval_end_listener(IntervalEndListener * listener) override;
    void set_reward_listener(RewardListener * listener) override;

    void restart(IntervalParams const& params) override;

    void note_arrival(Packet const * pkt) override;
    void note_drop(Packet const * pkt) override;
    void note_transmission(Packet const * pkt) override;

    auto get_current() const -> Queue * override;

    void write_stats(ostream& out) const override;
    void write_reward_stats(ostream& out) const override;

private:
    class IntervalTimerHandler;
    class SubintervalImterHandler;

private:
    auto is_switch_interval(size_t interval_idx) const -> bool;
    auto is_current_interval_switch() const -> bool;
    auto is_next_interval_switch() const -> bool;

    auto change_current(size_t idx) -> vector<Packet *>;

    void finish_interval();
    void start_interval();
    void restart_interval();

    void save_subinterval_reward();

private:
    vector<Queue *> const policies_;
    shared_ptr<schad::learning::LearningMethodFactory> const learning_factory_;

    unique_ptr<schad::learning::LearningMethod> learning_;
    optional<IntervalParams> interval_params_;

    unique_ptr<IntervalTimerHandler> interval_timer_;
    unique_ptr<SubintervalImterHandler> subinterval_timer_;

    size_t current_policy_idx_;

    unique_ptr<Reward> reward_;
    unique_ptr<Reward> subreward_;

    vector<double> subinterval_rewards_;

    size_t current_interval_idx_;

    IntervalEndListener * interval_end_listener_;
    RewardListener * reward_listener_;
};

#endif // NS_LEARNING_IMPL_H
