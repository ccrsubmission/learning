#ifndef LOCAL_GREEDY_H
#define LOCAL_GREEDY_H

#include <schad/learning/learning_method.h>

namespace schad::learning {

auto create_local_greedy() -> unique_ptr<LearningMethodFactory>;

}

#endif // RESTARTING_H
