#ifndef SOFTMAX_H
#define SOFTMAX_H

#include <schad/learning/learning_method.h>

namespace schad::learning {

auto create_softmax(double learning_rate) -> unique_ptr<LearningMethodFactory>;

}

#endif // SOFTMAX_H
