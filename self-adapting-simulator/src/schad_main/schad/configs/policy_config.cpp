#include <schad/policy/pq_policy.h>
#include "policy_config.h"

auto schad::load_policy(json const& cfg) -> shared_ptr<Policy> {
    auto const type = cfg.at("type").get<string>();
    if (type == "value") {
        return get_builtin_pq(BuiltInPQKind::VALUE);
    } else if (type == "work") {
        return get_builtin_pq(BuiltInPQKind::WORK);
    } else if (type == "value/work") {
        return get_builtin_pq(BuiltInPQKind::VALUE_PER_WORK);
    } else if (type == "value/slack") {
        return get_builtin_pq(BuiltInPQKind::VALUE_PER_SLACK);
    } else if (type == "deadline") {
        return get_builtin_pq(BuiltInPQKind::DEADLINE);
    } else {
        throw unknown_policy_exception(type);
    }
}

auto schad::load_policies(json const& cfg) -> vector<shared_ptr<Policy>> {
    vector<shared_ptr<Policy>> result(cfg.size());
    std::transform(cfg.begin(), cfg.end(), begin(result),
        [](auto const& pcfg) { 
            return load_policy(pcfg);
        }
    );
    return result;
}
