#ifndef UCB_E_H
#define UCB_E_H

#include <schad/learning/learning_method.h>

namespace schad::learning {

auto create_ucb_e(double a) -> unique_ptr<LearningMethodFactory>;

}

#endif // UCB_E_H
