#ifndef SCALED_H
#define SCALED_H

#include <schad/learning/learning_method.h>

namespace schad::learning {

auto create_scaled(shared_ptr<LearningMethodFactory> base, double factor) 
    -> unique_ptr<LearningMethodFactory>;

}

#endif // SCALED_H
