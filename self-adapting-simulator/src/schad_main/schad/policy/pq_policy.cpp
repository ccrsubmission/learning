#include <schad/policy/pq_policy_gen.h>
#include "pq_policy.h"

auto schad::get_builtin_pq(BuiltInPQKind kind) -> shared_ptr<Policy>const & {
    switch (kind) {
        case BuiltInPQKind::VALUE: {
            static shared_ptr<Policy> p = create_pq_policy("value", [] (auto const& x) { 
                 return -x.value();
            });
            return p;
        } break;
        case BuiltInPQKind::WORK: {
            static shared_ptr<Policy> p = schad::create_pq_policy("work", [] (auto const& x) { 
                return x.remaining_processing();
            });
            return p;
        } break;
        case BuiltInPQKind::VALUE_PER_WORK: {
            static shared_ptr<Policy> p = schad::create_pq_policy("value/work", [] (auto const& x) { 
                return -double(x.value()) / (double(x.remaining_processing()) + 0.0001);
            });
            return p;
        } break;
        case BuiltInPQKind::DEADLINE: {
            static shared_ptr<Policy> p = schad::create_pq_policy("deadline", [] (auto const& x) { 
                return deadline(x);
            });
            return p;
        } break;
        case BuiltInPQKind::VALUE_PER_SLACK: {
            static shared_ptr<Policy> p = schad::create_pq_policy("value/slack", [] (auto const t, auto const& x) { 
                return -x.value() / double(deadline(x) - t);
            });
            return p;
        } break;
        default: 
            exit(1);
    };
}

