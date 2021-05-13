#ifndef EXPERIMENT_CONFIG_H
#define EXPERIMENT_CONFIG_H

#include <schad/configs/loader.h>
#include <schad/simulator/experiment_parameters.h>

namespace schad {

auto load_experiment(json const& conf, Loader const& loader) 
    -> ExperimentParameters;
void to_json(json& j, ExperimentParameters const& exp);

}

#endif // EXPERIMENT_CONFIG_H
