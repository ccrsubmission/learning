#ifndef POLICY_CONFIG_H
#define POLICY_CONFIG_H

#include <nlohmann/json.hpp>
#include <schad/policy/policy.h>

namespace schad {

using json = nlohmann::json;

struct unknown_policy_exception : std::invalid_argument {
    unknown_policy_exception(string const& policy_name)
        : invalid_argument("unknown policy: " + policy_name) {
    }
};

auto load_policy(json const& cfg) -> shared_ptr<Policy>; 
auto load_policies(json const& cfg) -> vector<shared_ptr<Policy>>;

}

#endif // POLICY_CONFIG_H
