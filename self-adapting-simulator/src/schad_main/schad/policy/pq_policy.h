#ifndef PQ_POLICY_H
#define PQ_POLICY_H

#include <schad/policy/policy.h>

namespace schad {

enum class BuiltInPQKind {
    WORK, VALUE, VALUE_PER_WORK, VALUE_PER_SLACK, DEADLINE
};

auto get_builtin_pq(BuiltInPQKind kind) -> shared_ptr<Policy> const&;


}

#endif // PQ_POLICY_H
