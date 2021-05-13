#ifndef PQ_POLICY_GEN_H
#define PQ_POLICY_GEN_H

#include <schad/policy/detail/pq.h>

namespace schad {

template<class Priority >
auto create_pq_policy(string const& name, Priority prio) -> unique_ptr<Policy> {
    return make_unique<detail::PQPolicy<Priority>>(name, prio);
}

}

#endif // PQ_POLICY_GEN_H
