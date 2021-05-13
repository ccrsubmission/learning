#include "union_source.h"

namespace {

using namespace schad;

class UnionSource : public Source {
public:
    UnionSource(shared_ptr<rng_t> rng, vector<unique_ptr<Source>> srcs) 
        : rng_{rng}, srcs_{std::move(srcs)} {
    }

    auto next(uint32_t time) -> vector<unique_ptr<Packet>> override {
        vector<unique_ptr<Packet>> packets{};
        for (auto& src : srcs_) {
            for (auto& packet : src->next(time)) {
                packets.emplace_back(std::move(packet));
            }
        }
        std::shuffle(begin(packets), end(packets), *rng_);
        return packets;
    }

private:
    shared_ptr<rng_t> rng_;
    vector<unique_ptr<Source>> srcs_;
};

class UnionSourceFactory : public SourceFactory {
public:
    UnionSourceFactory(vector<unique_ptr<SourceFactory>> srcs)
        : srcs_{std::move(srcs)} {
    }

    auto instantiate(shared_ptr<rng_t> rng) const -> unique_ptr<Source> override {
        vector<unique_ptr<Source>> srcs{};
        for (auto const& factory : srcs_) {
            srcs.emplace_back(factory->instantiate(rng));
        }
        return make_unique<UnionSource>(std::move(rng), std::move(srcs));
    }

    void to_json(json& j) const override {
        j = {{"type", "union"}, {"parameters", json::array()}};
        for (auto i = 0u; i < srcs_.size(); ++i) {
            j.at("parameters")[i] = *srcs_[i];
        }
    }

private:
    vector<unique_ptr<SourceFactory>> srcs_;
};

}

auto schad::create_union_source(vector<unique_ptr<SourceFactory>> srcs) -> unique_ptr<SourceFactory> {
    return make_unique<UnionSourceFactory>(std::move(srcs));
}

