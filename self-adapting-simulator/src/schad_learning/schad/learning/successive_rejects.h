#ifndef SUCCESSIVE_REJECTS_H
#define SUCCESSIVE_REJECTS_H

#include <schad/learning/arm_identification_method.h>

namespace schad::learning {

auto create_successive_rejects() -> unique_ptr<ArmIdentificationMethodFactory>;

}

#endif // SUCCESSIVE_REJECTS_H
