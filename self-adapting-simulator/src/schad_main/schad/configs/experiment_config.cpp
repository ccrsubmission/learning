#include "experiment_config.h"
#include <schad/configs/policy_config.h>
#include <schad/configs/source_config.h>
#include <schad/learning/learning_config.h>
#include <schad/configs/reward_config.h>

namespace {

using namespace schad;

auto load_simulation_parameters(json const& conf) -> SimulationParameters {
    return SimulationParameters{}
        .set_num_time_steps(conf.at("num_time_steps"))
        .set_num_runs(conf.at("num_runs"))
        .set_stat_batch_size(conf.at("stat_batch_size"));
}

auto save_simulation_parameters(SimulationParameters const& params) -> json {
    return {
        {"num_time_steps", params.num_time_steps()},
        {"num_runs", params.num_runs()},
        {"stat_batch_size", params.stat_batch_size()}
    };
}

auto load_infrastructure_parameters(json const& conf) -> InfrastructureParameters {
    return InfrastructureParameters{}
        .set_buffer_size(conf.at("buffer_size"))
        .set_batch_size(conf.at("batch_size"))
        .set_num_simulated(conf.at("num_simulated"));
}

auto save_infrastructure_parameters(InfrastructureParameters const& params) -> json {
    return {
        {"buffer_size", params.buffer_size()},
        {"batch_size", params.batch_size()},
        {"num_simulated", params.num_simulated()}
    };
}

}

auto schad::load_experiment(json const& conf, Loader const& loader)
        -> ExperimentParameters {
    return ExperimentParameters {}
        .set_simulation_parameters(load_simulation_parameters(conf.at("simulation")))
        .set_infra_params(load_infrastructure_parameters(conf.at("infrastructure")))
        .set_policies(loader.load(load_policies, conf.at("policies")))
        .set_source(loader.load(load_source, conf.at("source")))
        .set_learning(loader.load(load_learning_method, conf.at("schad_learning")))
        .set_reward(loader.load(load_reward, conf.at("reward")));
}

void schad::to_json(json& j, ExperimentParameters const& exp) {
    json policies;
    for (auto const& p : exp.policies()) {
        policies.push_back(*p);
    }
    j = {
        {"simulation", save_simulation_parameters(exp.simulation_parameters())},
        {"infrastructure", save_infrastructure_parameters(exp.infra_params())},
        {"source", *exp.source()},
        {"schad_learning", *exp.learning()},
        {"policies", policies},
        {"reward", *exp.reward()}
    };
}
