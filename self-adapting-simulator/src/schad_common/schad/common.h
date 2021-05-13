#ifndef COMMON_H
#define COMMON_H

#include <cassert>
#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <iterator>
#include <string_view>
#include <tuple>
#include <optional>
#include <random>
#include <nlohmann/json.hpp>

namespace schad {

using json = nlohmann::json;
using rng_t = std::mt19937_64;
using std::unique_ptr;
using std::make_unique;
using std::string;
using std::vector;
using std::string_view;
using std::tuple;
using std::make_tuple;
using std::pair;
using std::make_pair;
using std::shared_ptr;
using std::make_shared;
using std::optional;

}

#endif // COMMON_H
