#ifndef EPSILON_GREEDY_PARAMTERS_H
#define EPSILON_GREEDY_PARAMTERS_H

#include <schad/learning/average.h>

namespace schad::learning {

struct EpsilonGreedyParameters {
    EpsilonGreedyParameters() : 
        average_{KeepAll{}}, epsilon_{0.0}, delta_{0.0}, sigma_{1.0} {
    }

    auto& set_epsilon(double epsilon) {
        epsilon_ = epsilon;
        return *this;
    }

    auto epsilon() const {
        return epsilon_;
    }

    auto& set_delta(double delta) {
        delta_ = delta;
        return *this;
    }

    auto delta() const {
        return delta_;
    }

    auto& set_sigma(double sigma) {
        sigma_ = sigma;
        return *this;
    }

    auto sigma() const {
        return sigma_;
    }

    auto& set_average(Average avg) {
        average_ = avg;
        return *this;
    }

    auto average() const {
        return average_;
    }
private:
    Average average_;
    double epsilon_;
    double delta_;
    double sigma_;
};

inline void to_json(json& j, EpsilonGreedyParameters const& p) {
    j = {{"average", p.average()},
         {"epsilon", p.epsilon()},
         {"sigma", p.sigma()},
         {"delta", p.delta()}};
}

};

#endif // EPSILON_GREEDY_PARAMTERS_H
