#include <schad/learning/constant.h>
#include <schad/learning/ucb.h>
#include <schad/learning/epsilon_greedy.h>
#include <schad/learning/combined.h>
#include <schad/learning/restarting.h>
#include <schad/learning/ucb_e.h>
#include <schad/learning/dgp_ucb.h>
#include <schad/learning/softmax.h>
#include <schad/learning/normalized.h>
#include <schad/learning/ucb_tuned.h>
#include <schad/learning/ucb_v.h>
#include <schad/learning/scaled.h>
#include <schad/learning/uniform_identification.h>
#include <schad/learning/successive_rejects.h>
#include <schad/learning/explore_exploit.h>
#include <schad/learning/local_greedy.h>
#include "learning_config.h"

namespace schad::learning {
    void from_json(json const& average, learning::Average& avg) {
        if (average.at("type") == "keep_all") {
            avg = learning::KeepAll{};
        } else if (average.at("type") == "exponential") {
            avg = learning::Exponential{average.at("gamma").get<double>()};
        } else if (average.at("type") == "sliding_window") {
            avg = learning::SlidingWindow{average.at("num_steps").get<size_t>()};
        } else {
            throw std::invalid_argument("Unknown UCB averager!");
        }
    }
}

auto schad::load_learning_method(json const& cfg, Loader const& loader) 
    -> shared_ptr<learning::LearningMethodFactory> {
    auto const type = cfg.at("type").get<string>();
    auto const& params = cfg.at("parameters");
    if (type == "ucb") {
        return learning::create_ucb(learning::UCBParameters{}
            .set_ksi(params.at("ksi"))
            .set_average(params.at("average"))
            .set_restricted_exploration(params.at("restricted_exploration"))
        );
    } else if (type == "epsilon_greedy") {
        return learning::create_epsilon_greedy(
            learning::EpsilonGreedyParameters{}
                .set_epsilon(params.at("epsilon"))
                .set_average(params.at("average"))
                .set_delta(params.at("delta"))
                .set_sigma(params.at("sigma"))
        );
    } else if (type == "constant") {
        return learning::create_constant(params.at("arm_idx"));
    } else if (type == "combined") {
        return learning::create_combined(
            loader.load(load_learning_method, params.at("exploiter")),
            loader.load(load_learning_method, params.at("explorer"))
        );
    } else if (type == "restarting") {
        return learning::create_restarting(
            loader.load(load_learning_method, params.at("base")),
            params.at("num_steps")
        );
    } else if (type == "ucb_e") {
        return learning::create_ucb_e(params.at("a"));
    } else if (type == "dgp_ucb") {
        optional<learning::Average> average{};
        if (!params.at("average").is_null()) {
            average = params.at("average").get<learning::Average>();
        }
        return learning::create_dgp_ucb(
            params.at("delta"), params.at("ksi"), average
        );
    } else if (type == "softmax") {
        return learning::create_softmax(params.at("alpha"));
    } else if (type == "normalized") {
        return learning::create_normalized(
                loader.load(load_learning_method, params.at("base")));
    } else if (type == "ucb_tuned") {
        return learning::create_ucb_tuned(params.at("average"));
    } else if (type == "ucb_v") {
        return learning::create_ucb_v(params.at("average"));
    } else if (type == "scaled") {
        return learning::create_scaled(
            loader.load(load_learning_method, params.at("base")),
            params.at("factor")
        );
    } else if (type == "explore_exploit") {
        return learning::create_explore_exploit(
            params.at("time_limit"),
            loader.load(load_arm_identification_method, params.at("base"))
        );
    } else if (type == "local_greedy") {
        return learning::create_local_greedy();
    } else {
        throw unknown_learning_method_exception(type);
    }
}

auto schad::load_arm_identification_method(json const& cfg, [[maybe_unused]] Loader const& loader) 
    -> shared_ptr<learning::ArmIdentificationMethodFactory> {
    auto const type = cfg.at("type").get<string>();
    [[maybe_unused]] auto const& params = cfg.at("parameters");
    if (type == "uniform") {
        return learning::create_uniform();
    } else if (type == "successive_rejects") {
        return learning::create_successive_rejects();
    } else {
        throw unknown_arm_identification_method_exception(type);
    }
}
