#ifndef NORMALIZED_H
#define NORMALIZED_H

#include <schad/learning/learning_method.h>

namespace schad::learning {

auto create_normalized(shared_ptr<LearningMethodFactory> base) 
    -> unique_ptr<LearningMethodFactory>;

}

#endif // NORMALIZED_H
