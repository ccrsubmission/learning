#ifndef UNIFORM_IDENTIFICATION_H
#define UNIFORM_IDENTIFICATION_H

#include <schad/learning/arm_identification_method.h>

namespace schad::learning {
auto create_uniform() -> unique_ptr<ArmIdentificationMethodFactory>;
} // namespace schad::learning
#endif
