#ifndef CONSTANT_H
#define CONSTANT_H

#include <schad/learning/learning_method.h>

namespace schad::learning {

auto create_constant(size_t arm_idx) -> unique_ptr<LearningMethodFactory>;

}

#endif // CONSTANT_H
