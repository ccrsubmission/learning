#ifndef SIMULATION_H
#define SIMULATION_H

#include <schad/simulator/experiment_parameters.h>
#include <schad/simulator/statistics.h>
#include <schad/common.h>


namespace schad {

auto run(ExperimentParameters const& parms) -> Statistics;

}

#endif // SIMULATION_H
