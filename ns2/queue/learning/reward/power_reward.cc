#include "power_reward.h"

#include <string>
#include <cmath>
#include <ostream>

static class PowerRewardClass : public TclClass {
public:
    PowerRewardClass() : TclClass("Reward/Power") { }

    TclObject *create(int argc, const char *const *argv) override {
        if (argc >= 10) {
            return new PowerReward{stof(argv[4]), stof(argv[5]), stof(argv[6]),
                                   stof(argv[7]), stof(argv[8]),
                                   stof(argv[9])};
        }
        return nullptr;
    }
} power_reward_class;

PowerReward::PowerReward(double bandwidth, 
                         double min_bw_fraction, double max_bw_fraction,
                         double min_delay, double max_delay, double delta)  
    : bandwidth_{bandwidth}, 
      min_bw_fraction_{min_bw_fraction}, max_bw_fraction_{max_bw_fraction},
      min_delay_{min_delay}, max_delay_{max_delay}, delta_{delta},
      interval_start_{}, flows_{} {
    reset({});
}

void PowerReward::note_arrival(Packet const * p) {
    if (flows_.find(HDR_IP(p)->flowid()) == end(flows_)) {
        flows_.emplace(HDR_IP(p)->flowid(), FlowStatistic{});
    }
    flows_.at(HDR_IP(p)->flowid()).note_arrival(p);
}

void PowerReward::note_drop(Packet const * p) {
    flows_.at(HDR_IP(p)->flowid()).note_drop(p);
}

void PowerReward::note_transmission(Packet const * p) {
    assert(flows_.count(HDR_IP(p)->flowid())); // TODO check that we are IP

    flows_.at(HDR_IP(p)->flowid()).note_transmission(p);
}

auto PowerReward::get_value() const -> double {
    if (flows_.size() == 0) {
        return 0.5;
    }

    auto const min_reward = 
        log(min_bw_fraction_ * bandwidth_ / 8.0 / flows_.size()) 
            - delta_ * log(max_delay_);
    auto const max_reward = 
        log(max_bw_fraction_ * bandwidth_ / 8.0 / flows_.size())
            - delta_ * log(min_delay_);

    double result = 0;
    
    for (auto const& [flow, stats] : flows_) {
        auto const value = get_reward(get_interval(), stats);
        if (value.has_value()) {
            result += *value;
        } else {
            result += min_reward;
        }
    }

    result /= flows_.size();
    result = (result - min_reward) / (max_reward - min_reward);

    return std::min(1.0, std::max(result, 0.0));
}

void PowerReward::write_stats(ostream& out, size_t interval_id) const {
    for (auto const& [flow, stats] : flows_) {
        out << interval_id << " " << flow  << " "
            << stats.get_total_bytes_transmitted() << " "
            << stats.get_active_interval(get_interval()) << " "
            << stats.get_avg_delay() << "\n";
    }
}

void PowerReward::reset(vector<Packet const*> packets) {
    interval_start_ = Scheduler::instance().clock();
    flows_.clear();

    for (auto const packet : packets) {
        note_arrival(packet);
    }
}

auto PowerReward::get_reward(double interval, const FlowStatistic &stats) const
        -> std::optional<double> {
    double const throughput = stats.get_throughput(interval);
    double const avg_delay = stats.get_avg_delay();

    if (throughput == 0) {
        return std::nullopt;
    }

    return log(throughput) - delta_ * log(std::max(avg_delay, min_delay_));
}

auto PowerReward::get_interval() const -> double {
    return Scheduler::instance().clock() - interval_start_;
}

auto PowerReward::clone() const -> unique_ptr<Reward> {
    return make_unique<PowerReward>(
            bandwidth_, min_bw_fraction_, max_bw_fraction_,
                         min_delay_, max_delay_, delta_);
}
