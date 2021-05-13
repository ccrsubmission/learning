#include "null_source.h"

namespace {
using namespace schad;

class NullSource : public Source {
public:
    auto next([[maybe_unused]] uint32_t time) 
            -> vector<unique_ptr<Packet>> override {
        return {};
    }
};

class NullSourceFactory : public SourceFactory {
public:
    auto instantiate([[maybe_unused]] std::shared_ptr<rng_t> rng) const 
            -> unique_ptr<Source> override {
        return make_unique<NullSource>();
    }

    void to_json(json& j) const override {
        j = {{"type", "null"}, {"parameters", nullptr}};
    }
};

}

auto schad::create_null_source() -> unique_ptr<SourceFactory> {
    return make_unique<NullSourceFactory>();
}

