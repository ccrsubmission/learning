#include "sequence_source.h"

namespace {

using namespace schad;

class SequenceSource : public Source {
public:
    SequenceSource(vector<size_t> num_steps, vector<unique_ptr<Source>> sources)
        : num_steps_{std::move(num_steps)}, sources_{std::move(sources)}, 
          current_source_{0}, num_since_switch_{0} {
    }

    auto next(uint32_t time) -> vector<unique_ptr<Packet>> override {
        if (num_since_switch_ == num_steps_[current_source_]) {
            current_source_ = (current_source_ + 1) % num_steps_.size();
            num_since_switch_ = 0;
        } 

        num_since_switch_++;

        return sources_[current_source_]->next(time);
    }

private:
    vector<size_t> const num_steps_;
    vector<unique_ptr<Source>> const sources_;
    size_t current_source_;
    size_t num_since_switch_;
};

class SequenceSourceFactory : public SourceFactory {
public:
    SequenceSourceFactory(SequenceSourceParameters params) 
        : params_{std::move(params)} {
    }

    auto instantiate(shared_ptr<rng_t> rng) const -> unique_ptr<Source> override {
        std::vector<unique_ptr<Source>> sources(params_.sources().size());
        std::transform(begin(params_.sources()), end(params_.sources()), begin(sources),
                [&rng](auto const& x) { return x->instantiate(rng); });
        return make_unique<SequenceSource>(params_.num_steps(), std::move(sources));
    }

    void to_json(json& j) const override {
        json params = json::array();
        for (auto i = 0u; i < params_.sources().size(); ++i) {
            params[i] = {
                {"num_steps", params_.num_steps()[i]},
                {"src", *params_.sources()[i]}
            };
        }
        j = {{"type", "sequence"}, {"parameters", params}};
    }

private:
    SequenceSourceParameters const params_;
};

}

auto schad::create_sequence_source(SequenceSourceParameters  params) 
        -> unique_ptr<SourceFactory> {
    return make_unique<SequenceSourceFactory>(std::move(params));
}

