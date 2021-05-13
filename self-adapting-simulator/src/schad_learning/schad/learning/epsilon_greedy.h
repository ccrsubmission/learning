#ifndef EPSILON_GREEDY_H
#define EPSILON_GREEDY_H

#include <schad/learning/learning_method.h>
#include <schad/learning/average.h>
#include <schad/learning/epsilon_greedy_parameters.h>

namespace schad::learning {

auto create_epsilon_greedy(EpsilonGreedyParameters const& params) 
    -> unique_ptr<LearningMethodFactory>;

}


#endif // EPSILON_GREEDY_H
