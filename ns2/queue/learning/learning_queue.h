#ifndef NS_LEARNING_QUEUE_H
#define NS_LEARNING_QUEUE_H

#include "queue.h"

#include <schad/learning/learning_method.h>
#include "reward.h"
#include "interval_selection/interval_selector.h"
#include "utils/drop_proxy.h"
#include "learning.h"

#include <vector>
#include <memory>

class LearningQueue : public Queue, public IntervalEndListener {
public:
    LearningQueue();
    ~LearningQueue() override;

    void enque(Packet* pkt) override;
    Packet* deque() override;
    void drop(Packet* pkt) override;
    void drop(Packet* pkt, const char *s) override;
    auto peek_packets() const -> vector<Packet const *> override; 

protected:
    void reset() override;
    int command(int argc, const char *const *argv) override;

private:
    void add_policy(Queue* policy);
    void set_reward(Reward* reward);
    void set_interval_selector(unique_ptr<IntervalSelector> selector);
    void set_learning_factory(shared_ptr<schad::learning::LearningMethodFactory> factory);

    void start_learning();

    Queue * get_current();
    Queue const * get_current() const;

    void interval_ended() override;
    void report_stats() const;

    static auto set_channel(
            Tcl_Channel &channel, 
            char const * channel_id, 
            char const * msg
            ) -> int;

private:
    unique_ptr<Learning> learning_;

    unique_ptr<IntervalSelector> interval_selector_;

    shared_ptr<schad::learning::LearningMethodFactory> learning_factory_;
    Reward * reward_;
    vector<Queue *> policies_;

    Tcl_Channel learning_channel_;
    Tcl_Channel reward_channel_;

    utils::DropProxy drop_proxy_;
};

#endif // NS_LEARNING_QUEUE_H
