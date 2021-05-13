#include "config.h"
#include "learning_queue.h"
#include "utils/queue_utils.h"
#include "interval_selection/fixed_interval_selector.h"

#include <schad/configs/loader.h>
#include <schad/learning/learning_config.h>

#include <iostream>
#include <sstream>
#include <vector>

static class LearningClass : public TclClass {
public:
    LearningClass() : TclClass("Queue/Learning") {}
    TclObject* create(int argc, const char * const * argv) override {
        return new LearningQueue{};
    }
} class_learning;

LearningQueue::LearningQueue()
    : Queue{},

      interval_selector_{},

      reward_{}, 
      policies_{}, 

      learning_channel_{nullptr},
      reward_channel_{nullptr},

      drop_proxy_{this} {
}

LearningQueue::~LearningQueue() {

}

void LearningQueue::reset() {
    Queue::reset();
    for (auto policy : policies_) {
        auto packets = utils::take_packets_and_reset(policy);
        assert(std::unique(begin(packets), end(packets)) == end(packets));
        std::for_each(begin(packets), end(packets), 
                      [this](auto p) { drop(p); });
    }

    if (!policies_.empty() && learning_) {
        //TODO: restart_interval();
    }
}

int LearningQueue::command(int argc, const char *const *argv) {
    auto& tcl = Tcl::instance();

    if (argc == 3) {
        if (strcmp(argv[1], "set_interval_selector") == 0) {
            auto selector = (IntervalSelector*) TclObject::lookup(argv[2]);
            if (selector == nullptr) {
                tcl.add_errorf("no such object %s", argv[2]);
                return TCL_ERROR;
            }
            set_interval_selector(unique_ptr<IntervalSelector>(selector));
            return TCL_OK;
        }
        if (strcmp(argv[1], "add_policy") == 0) {
            auto policy = (Queue*) TclObject::lookup(argv[2]);
            if (policy == nullptr) {
                tcl.add_errorf("no such object %s", argv[2]);
                return TCL_ERROR;
            }
            add_policy(policy);
            return TCL_OK;
        }
        if (strcmp(argv[1], "set_reward") == 0) {
            auto reward = (Reward *) TclObject::lookup(argv[2]);
            if (reward == nullptr) {
                tcl.add_errorf("no such object %s", argv[2]);
                return TCL_ERROR;
            }
            set_reward(reward);
            return TCL_OK;
        }
        if (strcmp(argv[1], "attach") == 0) {
            return set_channel(learning_channel_, argv[2], "Learning trace");
        }
        if (strcmp(argv[1], "attach-reward") == 0) {
            return set_channel(reward_channel_, argv[2], "Reward trace");
        }
        if (strcmp(argv[1], "set_learning") == 0) {
            auto factory = schad::Loader::load_from_dir(schad::load_learning_method, argv[2]);
            if (!factory) {
                tcl.add_errorf("Learning: cannot load method");
                return TCL_ERROR;
            }
            set_learning_factory(move(factory));
            return TCL_OK;
        }
    }

    if (argc == 2) {
        if (strcmp(argv[1], "start_learning") == 0) {
            if (!learning_factory_) {
                tcl.add_errorf("ERROR start_learning: no learning method factory");
                return TCL_ERROR;
            }
            if (!reward_) {
                tcl.add_errorf("ERROR start_learning: no reward");
                return TCL_ERROR;
            }
            if (policies_.empty()) {
                tcl.add_errorf("ERROR start_learning: no policies");
                return TCL_ERROR;
            }
            if (!interval_selector_) {
                tcl.add_errorf("ERROR start_learning: no interval selector");
                return TCL_ERROR;
            }
            start_learning();
            return TCL_OK;
        }
    }

    return Queue::command(argc, argv);
}

void LearningQueue::enque(Packet *pkt) {
    if (!policies_.empty()) {
        if (learning_) {
            learning_->note_arrival(pkt);
        }
        get_current()->enque(pkt);
    } else {
        drop(pkt);
    }
}

Packet *LearningQueue::deque() {
    if (!policies_.empty()) {
        auto const packet = get_current()->deque();
        if (packet != nullptr && learning_) {
            learning_->note_transmission(packet);
        }
        return packet;
    }

    return nullptr;
}

void LearningQueue::add_policy(Queue *policy) {
    auto& tcl = Tcl::instance();

    tcl.evalf("%s set limit_ %d", policy->name(), qlim_);
    policy->setDropTarget(&drop_proxy_);

    policies_.push_back(policy);
}

void LearningQueue::drop(Packet *pkt) {
    if (learning_) {
        learning_->note_drop(pkt);
    }

    Queue::drop(pkt);
}

void LearningQueue::drop(Packet *pkt, const char *s) {
    if (reward_) {
        reward_->note_drop(pkt);
    }

    Queue::drop(pkt, s);
}

void LearningQueue::set_reward(Reward *reward) {
    reward_ = reward;
}

void LearningQueue::set_learning_factory(
        shared_ptr<schad::learning::LearningMethodFactory> factory
        ) {
    learning_factory_ = move(factory);
}


Queue * LearningQueue::get_current() {
    if (learning_) {
        return learning_->get_current();
    } else {
        return policies_.front();
    }
}

Queue const * LearningQueue::get_current() const {
    if (learning_) {
        return learning_->get_current();
    } else {
        return policies_.front();
    }
}

void LearningQueue::report_stats() const {
    if (learning_channel_) {
        ostringstream out{};
        learning_->write_stats(out);
        Tcl_Write(learning_channel_, out.str().c_str(), -1);
    }
    if (reward_channel_) {
        ostringstream out{};
        learning_->write_reward_stats(out);
        Tcl_Write(reward_channel_, out.str().c_str(), -1);
    }
}

auto LearningQueue::set_channel(Tcl_Channel &channel, char const * channel_id,
                                char const * msg) -> int {
    auto& tcl = Tcl::instance();

    int mode;
    channel = Tcl_GetChannel(tcl.interp(), channel_id, &mode);
    if (!channel) {
        tcl.resultf("%s: can't attach %s for writing", msg, channel_id);
        return TCL_ERROR;
    }
    return TCL_OK;
}

void LearningQueue::set_interval_selector(unique_ptr<IntervalSelector> selector) {
    interval_selector_ = move(selector);
}

void LearningQueue::start_learning() {
    interval_selector_->reset(policies_.size());
    learning_ = Learning::build(policies_, learning_factory_, *reward_);
    learning_->set_interval_end_listener(this);
    learning_->set_reward_listener(interval_selector_.get());
    learning_->restart(*interval_selector_->take_new_params());
}

auto LearningQueue::peek_packets() const -> vector<Packet const *> {
    return get_current()->peek_packets();
}

void LearningQueue::interval_ended() {
    report_stats();
    auto new_params = interval_selector_->take_new_params();
    if (new_params) {
        learning_->restart(*new_params);
    }
}
