#include <iostream>
#include <iomanip>

#include <schad/infrastructure/infrastructure.h>
#include <schad/simulator/multi_run_stats_collector.h>
#include "simulation.h"

namespace {

using namespace schad;

class Simulator : public World {
public:
    Simulator(
            SimulationParameters const& params, 
            InfrastructureParameters const& infra_params,
            learning::LearningMethodFactory const& learning,
            vector<shared_ptr<Policy>> const& policies, 
            RewardFunction const& reward,
            StatsCollector * stats,
            unique_ptr<Source> src,
            shared_ptr<rng_t> rng)
        : params_{params}, current_step_{0}, rng_{rng},
          src_{std::move(src)},
          infra_{create_infastructure(
               infra_params, reward, policies, learning, this, stats
          )} {
    }

    void run() {
        for (auto i = 0u; i < params_.num_time_steps(); ++i) {
            run_step();
        }
    }

    auto current_time() const -> uint32_t override {
        return current_step_;
    }

    auto rng() -> shared_ptr<rng_t> const& override {
        return rng_;
    }

private:
    void run_step() {
        auto packets = src_->next(current_step_);
        for (auto [p,r] : infra_->simulated_policies()) {
            vector<unique_ptr<Packet>> packets_clone(packets.size());
            std::transform(begin(packets), end(packets), begin(packets_clone), 
                [](auto const& p) {
                    return p->clone();
                }
            );

            process_admission(*p, *r, std::move(packets_clone));
        }
        auto [active_policy, active_reward] = infra_->active_policy();
        process_admission(*active_policy, *active_reward, std::move(packets));

        infra_->tick();
        current_step_++;
    }

    void process_admission(PolicyInstance &policy, RewardFunction &reward, 
                vector<unique_ptr<Packet>> packets) {
        for (auto const& p : packets) {
            reward.note_arrival(*p);
        }
        policy.admit_n_drop(std::move(packets));

        auto next_to_process = policy.select_for_processing();
        if (next_to_process != nullptr) {
            next_to_process->process();
            policy.processing_finished(next_to_process);
        }

        auto transmitted = policy.transmit();
        if (transmitted) {
            reward.note_transmission(current_step_, *transmitted);
        }
    }

private:
    SimulationParameters const params_; 
    uint32_t current_step_;

    shared_ptr<rng_t> const rng_;
    unique_ptr<Source> const src_;
    unique_ptr<Infrastructure> const infra_; 
};


}

auto schad::run(ExperimentParameters const& params) -> Statistics {
    auto internal_rng = make_shared<rng_t>(
        params.simulation_parameters().seed()
    );
    auto external_rng = make_shared<rng_t>((*internal_rng)());

    MultiRunStatsCollector stats{
        params.policies().size(), 
        params.simulation_parameters().stat_batch_size()
    }; 

    for (auto i = 0u; i < params.simulation_parameters().num_runs(); ++i) {
        std::cerr  << "Run #" << std::setw(5) << (i + 1) << " of " 
            << std::setw(5) << params.simulation_parameters().num_runs();
        if (i + 1 == params.simulation_parameters().num_runs()) {
            std::cerr << "\n";
        } else {
            std::cerr << "\r";
        }

        stats.next_run();
        Simulator{
            params.simulation_parameters(), params.infra_params(), 
            *params.learning(), params.policies(), *params.reward(), &stats,
            params.source()->instantiate(external_rng), internal_rng
        }.run();
    }

    return stats.get_statistics();
}
