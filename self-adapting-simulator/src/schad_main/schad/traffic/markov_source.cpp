#include "markov_source.h"

namespace {
using namespace schad;

class MarkovSource : public Source {
public:
    MarkovSource(vector<std::discrete_distribution<size_t>> tr, 
                 vector<std::unique_ptr<Source>> srcs,
                 size_t initial_state,
                 shared_ptr<rng_t> rng)
        : tr_{std::move(tr)}, srcs_{std::move(srcs)}, current_state_{initial_state},
          rng_{std::move(rng)} {
    }

    auto next(uint32_t time) -> vector<unique_ptr<Packet>> override {
        auto result = srcs_[current_state_]->next(time);
        current_state_ = tr_[current_state_](*rng_);
        return result;
    }

private:
    vector<std::discrete_distribution<size_t>> tr_;
    vector<std::unique_ptr<Source>> const srcs_;
    size_t current_state_;

    shared_ptr<rng_t> const rng_;
};

class MarkovSourceFactory : public SourceFactory {
public:
    MarkovSourceFactory(MarkovSourceParameters params)
        : params_{std::move(params)} {
    }

    auto instantiate(shared_ptr<rng_t> rng) const -> unique_ptr<Source> override {
        vector<std::discrete_distribution<size_t>> tr{};
        vector<std::unique_ptr<Source>> srcs{};

        for (auto const& s : params_.states()) {
            srcs.emplace_back(s.src()->instantiate(rng));

            vector<double> weights(params_.states().size(), 0.0);
            std::transform(
                begin(params_.states()), end(params_.states()), begin(weights),
                [&s] (auto const& to) { return s.tr().at(to.name()); }
            );

            tr.emplace_back(begin(weights), end(weights));
        }
        auto initial_state = std::distance(begin(params_.states()), std::find_if(
                begin(params_.states()), end(params_.states()),
                [] (auto const& x) { return x.is_start(); }));
        assert(initial_state < int(params_.states().size()));

        return make_unique<MarkovSource>(
            tr, std::move(srcs), initial_state, std::move(rng)
        );
    }

    void to_json(json& j) const override {
        j = {{"type", "markov"}, {"parameters", params_}};
    }

private:
    MarkovSourceParameters const params_;
};

}

auto schad::create_markov_source(MarkovSourceParameters params) 
        -> unique_ptr<SourceFactory> {
    return std::make_unique<MarkovSourceFactory>(std::move(params));
}
