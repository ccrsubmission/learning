#ifndef RESTARTING_H
#define RESTARTING_H

#include <schad/learning/learning_method.h>

namespace schad::learning {

auto create_restarting(shared_ptr<LearningMethodFactory> base, size_t num_steps) 
    -> unique_ptr<LearningMethodFactory>;

}

#endif // RESTARTING_H
