#include "source_config.h"
#include <schad/traffic/poisson_source.h>
#include <schad/traffic/union_source.h>
#include <schad/traffic/sequence_source.h>
#include <schad/traffic/markov_source.h>
#include <schad/traffic/null_source.h>

auto schad::load_source(json const& cfg, Loader const& loader) 
        -> unique_ptr<SourceFactory> {
    if (cfg.at("type") == "poisson") {
        auto const& params = cfg.at("parameters");
        auto ps_params = PoissonSourceParameters()
                .set_min_value(params.at("min_value"))
                .set_max_value(params.at("max_value"))
                .set_min_work(params.at("min_work"))
                .set_max_work(params.at("max_work"))
                .set_rate(params.at("rate"));

        if (params.count("slack")) {
            ps_params.set_min_slack(params["slack"].at("min"));
            ps_params.set_max_slack(params["slack"].at("max"));
        }

        return create_poisson_source(std::move(ps_params));
    } else if (cfg.at("type") == "union") { 
        vector<unique_ptr<SourceFactory>> sub_sources{};
        for (auto const& sub_source : cfg.at("parameters")) {
            sub_sources.emplace_back(loader.load(load_source, sub_source));
        }
        return create_union_source(std::move(sub_sources));
    } else if (cfg.at("type") == "sequence") {
        auto const& params = cfg.at("parameters");
        auto src_params = SequenceSourceParameters();
        for (auto it = params.begin(); it != params.end(); ++it) {
            src_params.add_source(
                it.value().at("num_steps"),
                loader.load(load_source, it.value().at("src"))
            );
        }
        return create_sequence_source(std::move(src_params));
    } else if (cfg.at("type") == "markov") {
        auto const& params = cfg.at("parameters");
        auto markov_params = MarkovSourceParameters{};
        for (auto it = params.begin(); it != params.end(); ++it) {
            auto state = MarkovState{
                    it.key(), 
                    loader.load(load_source, it.value().at("src")), 
                    it.value().at("is_start").get<bool>()
            };
            auto const& tr = it.value().at("tr");
            for (auto tr_it = tr.begin(); tr_it != tr.end(); ++tr_it) {
                state.add_transition(tr_it.key(), tr_it.value());
            }
            markov_params.add_state(std::move(state));
        }
        return create_markov_source(std::move(markov_params));
    } else if (cfg.at("type") == "null") {
        return create_null_source();
    } else {
        throw unknown_source_exception(cfg.at("type").get<string>());
    }
}


