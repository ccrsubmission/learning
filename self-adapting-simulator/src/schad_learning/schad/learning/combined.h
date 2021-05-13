#ifndef COMBINED_H
#define COMBINED_H

#include <schad/learning/learning_method.h>

namespace schad::learning {

auto create_combined(
        shared_ptr<LearningMethodFactory> exploiter,
        shared_ptr<LearningMethodFactory> explorer) 
    -> unique_ptr<LearningMethodFactory>;

}

#endif
