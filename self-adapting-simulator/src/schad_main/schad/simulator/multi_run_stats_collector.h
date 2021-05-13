#ifndef MULTI_RUN_STATS_COLLECTOR_H
#define MULTI_RUN_STATS_COLLECTOR_H

#include <schad/simulator/statistics.h>
#include <schad/infrastructure/stats_collector.h>

namespace schad {

class MultiRunStatsCollector : public StatsCollector {
public:
    MultiRunStatsCollector(size_t num_arms, uint64_t batch_size);

    void next_run();
    void append_step(size_t arm_idx, Reward reward) override;

    auto get_statistics() const -> Statistics;

private:
    size_t const num_arms_;
    uint64_t const batch_size_;
    uint64_t batch_idx_;
    vector<vector<double>> rewards_;
    vector<vector<double>> totals_;
    vector<vector<vector<size_t>>> arms_;
};

}

#endif // MULTI_RUN_STATS_COLLECTOR_H
