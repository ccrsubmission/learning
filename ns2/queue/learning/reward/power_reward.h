#ifndef NS_POWER_REWARD_H
#define NS_POWER_REWARD_H

#include "ip.h"

#include "reward.h"
#include "flow_statistics.h"

#include <unordered_map>

class PowerReward : public Reward {
public:
    PowerReward(double bandwidth, 
                double min_bw_fraction, double max_bw_fraction,
                double min_delay, double max_delay,
                double delta);

    void note_arrival(Packet const * p) override;

    void note_drop(Packet const * p) override;

    void note_transmission(Packet const * p) override;

    auto get_value() const -> double override;

    void reset(vector<Packet const*> packets) override;

    void write_stats(ostream& out, size_t interval_idx) const override;

    auto clone() const -> unique_ptr<Reward> override;

private:
    auto get_interval() const -> double;
    auto get_reward(double interval, FlowStatistic const& stats) const
        -> std::optional<double>;

private:
    double const bandwidth_;
    double const min_bw_fraction_;
    double const max_bw_fraction_;
    double const min_delay_;
    double const max_delay_;
    double const delta_;

    double interval_start_;
    std::unordered_map<FlowID, FlowStatistic> flows_;

    Tcl_Channel trace_channel_;
};


#endif // NS_POWER_REWARD_H
