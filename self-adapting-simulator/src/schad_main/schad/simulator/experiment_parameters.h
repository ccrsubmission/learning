#ifndef EXPERIMENT_PARAMETERES_H
#define EXPERIMENT_PARAMETERES_H

#include <schad/infrastructure/infrastructure_parameters.h>
#include <schad/traffic/source.h>
#include <schad/learning/learning_method.h>
#include <schad/policy/policy.h>
#include <schad/reward/reward_function.h>
#include <schad/simulator/simulation_parameters.h>

namespace schad {

struct ExperimentParameters {
    ExperimentParameters() 
        : sim_params_{}, infra_params_{}, 
          learning_{}, policies_{}, reward_{},
          source_{} {
    }

    auto& set_simulation_parameters(SimulationParameters params) {
        sim_params_ = params;
        return *this;
    }

    auto& simulation_parameters() const {
        return sim_params_;
    }

    auto& set_infra_params(InfrastructureParameters infra_params) {
        infra_params_ = infra_params;
        return *this;
    }

    auto& infra_params() const {
        return infra_params_;
    }

    auto& set_policies(vector<shared_ptr<Policy>> policies) {
        policies_ = policies;
        return *this;
    }

    auto& policies() const {
        return policies_;
    }

    auto& set_learning(std::shared_ptr<learning::LearningMethodFactory> learning) {
        learning_ = learning;
        return *this;
    }

    auto& learning() const {
        return learning_;
    }

    auto& set_source(shared_ptr<SourceFactory> source) {
        source_ = std::move(source);
        return *this;
    }

    auto& source() const {
        return source_;
    }

    auto& set_reward(shared_ptr<RewardFunction const> reward) {
        reward_ = reward;
        return *this;
    }

    auto& reward() const {
        return reward_;
    }

private:
    SimulationParameters sim_params_;
    InfrastructureParameters infra_params_;

    std::shared_ptr<learning::LearningMethodFactory> learning_;
    vector<shared_ptr<Policy>> policies_;
    std::shared_ptr<RewardFunction const> reward_;

    shared_ptr<SourceFactory> source_;
};

}


#endif // ExperimentParameters
