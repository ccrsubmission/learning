#ifndef UCB_TUNED_H
#define UCB_TUNED_H

#include <schad/learning/learning_method.h>
#include <schad/learning/average.h>

namespace schad::learning {

auto create_ucb_tuned(Average avg) -> unique_ptr<LearningMethodFactory>;

}

#endif // UCB_TUNED_H
