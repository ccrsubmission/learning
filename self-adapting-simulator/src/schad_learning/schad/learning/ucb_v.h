#ifndef UCB_V_H
#define UCB_V_H

#include <schad/learning/learning_method.h>
#include <schad/learning/average.h>

namespace schad::learning {

auto create_ucb_v(Average avg) -> unique_ptr<LearningMethodFactory>;

}

#endif // UCB_V_H
