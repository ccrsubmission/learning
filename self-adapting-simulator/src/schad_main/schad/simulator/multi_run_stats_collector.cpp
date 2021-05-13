#include "multi_run_stats_collector.h"

namespace schad {

MultiRunStatsCollector::MultiRunStatsCollector(
        size_t num_arms, uint64_t batch_size) 
    : num_arms_{num_arms}, batch_size_{batch_size}, batch_idx_{0},
      rewards_{}, totals_{}, arms_{} {
}

void MultiRunStatsCollector::append_step(size_t arm_idx, Reward reward) {
    assert(!rewards_.empty() && !arms_.empty());

    if (batch_idx_ == 0) {
        rewards_.back().push_back(0.0);
        totals_.back().push_back(0.0);
        for (auto& arm : arms_.back()) {
            arm.push_back(0);
        }
    }

    rewards_.back().back() += reward.value();
    totals_.back().back() += reward.total().value_or(0.0);
    arms_.back()[arm_idx].back()++;

    batch_idx_++;
    if (batch_idx_ == batch_size_) {
        batch_idx_ = 0;
    }
}

void MultiRunStatsCollector::next_run() {
    rewards_.push_back({});
    totals_.push_back({});
    arms_.push_back(vector<vector<size_t>>(num_arms_));
    batch_idx_ = 0;
}

auto MultiRunStatsCollector::get_statistics() const -> Statistics {
    return Statistics{rewards_, totals_, arms_};
}

}
