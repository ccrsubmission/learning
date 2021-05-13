#ifndef WORLD_H
#define WORLD_H

#include <schad/common.h>

namespace schad {

struct World {
    virtual auto current_time() const -> uint32_t = 0;
    virtual auto rng() -> shared_ptr<rng_t> const& = 0;

    World() = default;
    World(World const&) = delete; 
    World& operator=(World const&) = delete;
    
    virtual ~World() = default;
};

}

#endif // WORLD_H
