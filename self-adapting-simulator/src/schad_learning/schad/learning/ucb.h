#ifndef UCB_H
#define UCB_H

#include <schad/learning/learning_method.h>
#include <schad/learning/ucb_parameters.h>

namespace schad::learning {

auto create_ucb(UCBParameters const& params) -> unique_ptr<LearningMethodFactory>;

}

#endif // UCB_H
