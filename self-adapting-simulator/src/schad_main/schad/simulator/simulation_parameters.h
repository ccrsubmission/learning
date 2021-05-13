#ifndef SIMULATION_PARAMETERS_H
#define SIMULATION_PARAMETERS_H

#include <schad/common.h>

namespace schad {

struct SimulationParameters {
    SimulationParameters()
        : num_time_steps_{100}, num_runs_{1}, stat_batch_size_{1}, seed_{42} {
    }

    auto& set_num_runs(uint64_t num_runs) {
        num_runs_ = num_runs;
        return *this;
    }

    auto num_runs() const {
        return num_runs_;
    }

    auto& set_num_time_steps(uint64_t num_time_steps) {
        num_time_steps_ = num_time_steps;
        return *this;
    }

    auto num_time_steps() const {
        return num_time_steps_;
    }

    auto& set_seed(uint64_t seed) {
        seed_ = seed;
        return *this;
    }

    auto seed() const {
        return seed_;
    }

    auto& set_stat_batch_size(uint64_t n) {
        stat_batch_size_ = n;
        return *this;
    }

    auto stat_batch_size() const {
        return stat_batch_size_;
    }

private:
    uint64_t num_time_steps_;
    uint64_t num_runs_;
    uint64_t stat_batch_size_;
    uint64_t seed_;
};

}


#endif // SIMULATION_PARAMETERS_H
