#include "learning_impl.h"
#include "timer-handler.h"

#include "utils/queue_utils.h"

#include <schad/reward/reward.h>

#include <iostream>

class LearningImpl::IntervalTimerHandler : public TimerHandler {
public:
    explicit IntervalTimerHandler(LearningImpl * learning) : learning_{learning} {}

protected:
    void expire(Event *event) override {
        learning_->restart_interval();
    }

private:
    LearningImpl * learning_;
};

class LearningImpl::SubintervalImterHandler : public TimerHandler {
public:
    explicit SubintervalImterHandler(LearningImpl * learning) : learning_{learning} {}

protected:
    void expire(Event *event) override {
        learning_->save_subinterval_reward();
    }

private:
    LearningImpl * learning_;
};


LearningImpl::LearningImpl(
        vector<Queue *> policies, 
        shared_ptr<schad::learning::LearningMethodFactory> learning_factory,
        Reward const& reward
        ) 
    : policies_{move(policies)}
    , learning_factory_{move(learning_factory)}
    , learning_{}
    , interval_params_{}
    , interval_timer_{make_unique<IntervalTimerHandler>(this)} 
    , subinterval_timer_{make_unique<SubintervalImterHandler>(this)}
    , current_policy_idx_{0}
    , reward_{reward.clone()}
    , subreward_{reward.clone()}
    , subinterval_rewards_{}
    , current_interval_idx_{0}
    , interval_end_listener_{nullptr}
    , reward_listener_{nullptr}
{}

LearningImpl::~LearningImpl() {

}

void LearningImpl::restart(IntervalParams const& params) {
    assert(interval_timer_->status() != TimerHandler::TIMER_PENDING);
    assert(subinterval_timer_->status() == TimerHandler::TIMER_IDLE);

    learning_ = learning_factory_->instantiate(
        make_shared<schad::rng_t>(), policies_.size()
    );
    interval_params_ = move(params);
    current_interval_idx_ = 0;

    start_interval();
}

void LearningImpl::start_interval() {
    if (is_current_interval_switch()) {
        auto buffered = change_current(learning_->choose().front());
        reward_->reset(vector<Packet const*>(begin(buffered), end(buffered)));
    } else {
        reward_->reset(get_current()->peek_packets());
    }
    interval_timer_->resched(interval_params_->interval());

    subreward_->reset(get_current()->peek_packets());
    subinterval_rewards_.clear();
    subinterval_timer_->resched(interval_params_->subinterval());
}

void LearningImpl::finish_interval() {
    if (is_next_interval_switch() && current_interval_idx_ != 0) {
        vector<optional<schad::Reward>> rewards(policies_.size(), nullopt);

        rewards[current_policy_idx_] = reward_->get_value();
        subinterval_rewards_.push_back(subreward_->get_value());

        learning_->report_rewards(rewards);

        if (reward_listener_) {
            reward_listener_->report_rewards(
                current_policy_idx_, 
                reward_->get_value(), 
                subinterval_rewards_);
        }
        if (interval_end_listener_) {
            interval_end_listener_->interval_ended();
        }
    }

    current_interval_idx_++;
}

void LearningImpl::restart_interval() {
    finish_interval();
    start_interval();
}

void LearningImpl::save_subinterval_reward() {
    if (subinterval_rewards_.size() + 1 < interval_params_->num_subintervals()) {
        subinterval_rewards_.push_back(subreward_->get_value());
        subreward_->reset(get_current()->peek_packets());
    }
    if (subinterval_rewards_.size() + 1 < interval_params_->num_subintervals()) {
        subinterval_timer_->resched(interval_params_->subinterval());
    }
}

auto LearningImpl::get_current() const -> Queue * {
    return policies_[current_policy_idx_];
}

auto LearningImpl::change_current(size_t new_idx) -> vector<Packet *> {
    // TODO: what if the same
    auto packets = utils::take_packets_and_reset(get_current());
    current_policy_idx_ = new_idx;
    utils::init_queue_with(get_current(), packets);
    return packets;
}

auto LearningImpl::is_next_interval_switch() const -> bool {
    return interval_params_->is_switch_interval(current_interval_idx_ + 1);
}

auto LearningImpl::is_current_interval_switch() const -> bool {
    return interval_params_->is_switch_interval(current_interval_idx_);
}

void LearningImpl::write_stats(ostream& out) const {
    out << current_interval_idx_ << " " << reward_->get_value() 
        << " " << interval_params_->interval();
    for (auto subr : subinterval_rewards_) {
        out << " " << subr;
    }
    out << "\n";
}

void LearningImpl::write_reward_stats(ostream& out) const {
    reward_->write_stats(out, current_interval_idx_);
}

void LearningImpl::note_arrival(Packet const * pkt) {
    reward_->note_arrival(pkt);
    subreward_->note_arrival(pkt);
}

void LearningImpl::note_drop(Packet const * pkt) {
    reward_->note_drop(pkt);
    subreward_->note_drop(pkt);
}

void LearningImpl::note_transmission(Packet const * pkt) {
    reward_->note_transmission(pkt);
    subreward_->note_transmission(pkt);
}

void LearningImpl::set_interval_end_listener(IntervalEndListener * listener) {
    interval_end_listener_ = listener;
}

void LearningImpl::set_reward_listener(RewardListener * listener) {
    reward_listener_ = listener;
}
