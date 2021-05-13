#ifndef SOURCE_H
#define SOURCE_H

#include <schad/packet/packet.h>

namespace schad {

struct Source {
    virtual auto next(uint32_t time) -> vector<unique_ptr<Packet>> = 0;

    Source() = default;
    Source(Source const&) = delete;
    Source& operator=(Source const&) = delete;

    virtual ~Source() = default;
};

struct SourceFactory {
    virtual auto instantiate(shared_ptr<rng_t> rng) const -> unique_ptr<Source> = 0;

    virtual void to_json(json& json) const = 0;

    SourceFactory() = default;
    SourceFactory(SourceFactory const&) = delete;
    SourceFactory& operator=(SourceFactory const&) = delete;

    virtual ~SourceFactory() = default;
};

inline void to_json(json& j, SourceFactory const& source) {
    source.to_json(j);
}

}

#endif // SOURCE_H
