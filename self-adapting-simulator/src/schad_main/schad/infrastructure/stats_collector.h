#ifndef STATS_COLLECTOR_H
#define STATS_COLLECTOR_H

#include <schad/reward/reward.h>

namespace schad {

struct StatsCollector {
    virtual void append_step(size_t arm_idx, Reward reward) = 0;

    StatsCollector() = default;
    StatsCollector(StatsCollector const&) = delete;
    StatsCollector& operator=(StatsCollector const&) = delete;

    virtual ~StatsCollector() = default;
};

}

#endif // STATS_COLLECTOR_H
