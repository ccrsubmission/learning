#include <boost/program_options.hpp>

#include <schad/learning/learning_config.h>
#include <schad/configs/policy_config.h>
#include <schad/configs/source_config.h>
#include <schad/configs/experiment_config.h>
#include <schad/simulator/simulation.h>
#include <schad/reward/weighted_throughput_reward.h>

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <memory>

int main(int argc, char const * argv[]) {
    namespace po = boost::program_options;

    po::options_description desc("Scheduler Adaptive options");
    desc.add_options()
        ("help", "produce help message")
        ("buffer-size", po::value<size_t>()->default_value(10), 
            "Buffer size")
        ("num-time-steps", po::value<size_t>()->default_value(10), 
            "Number of simulation steps")
        ("num-runs", po::value<size_t>()->default_value(100),
            "Number of simulation runs")
        ("source-config", po::value<std::string>()->default_value("source.json"), 
            "Source JSON configuration")
        ("policies-config", po::value<std::string>()->default_value("policies.json"),
            "Policies JSON configuration")
        ("schad_learning-config", po::value<std::string>()->default_value("schad_learning.json"),
            "Learning JSON configuration")
        ("batch-size", po::value<size_t>()->default_value(1000),
            "Batch size")
        ("num-simulated", po::value<size_t>()->default_value(0),
            "Number of simulated policies")
        ("experiment-config", po::value<std::string>(),
            "Experiment JSON configuration (takes precedence)")
        ;

    po::variables_map vm{};
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    auto experiment = schad::ExperimentParameters{};
    if (vm.count("experiment-config")) {
        experiment = schad::Loader::load_from_dir(
            schad::load_experiment, vm["experiment-config"].as<std::string>()
        );
    } else {
        experiment.set_source(schad::Loader::load_from_dir(
            schad::load_source, vm["source-config"].as<std::string>()))
        .set_policies(schad::Loader::load_from_dir(
            schad::load_policies, vm["policies-config"].as<std::string>()))
        .set_learning(schad::Loader::load_from_dir(
            schad::load_learning_method, vm["schad_learning-config"].as<std::string>()))
        .set_simulation_parameters(schad::SimulationParameters{}
            .set_num_time_steps(vm["num-time-steps"].as<size_t>())
            .set_num_runs(vm["num-runs"].as<size_t>()))
        .set_infra_params(schad::InfrastructureParameters()
            .set_buffer_size(vm["buffer-size"].as<size_t>())
            .set_batch_size(vm["batch-size"].as<size_t>())
            .set_num_simulated(vm["num-simulated"].as<size_t>()))
        .set_reward(schad::rewards::weighted_throughput())
        ;
    }

    schad::json json_output{};
    json_output["experiment"] = experiment;
    json_output["result"] = schad::run(experiment);

    std::cout << json_output;
    return 0;
}

