#ifndef POLICY_H
#define POLICY_H

#include <schad/packet/packet.h>
#include <schad/simulator/world.h>

namespace schad {

struct PolicyInstance {
    virtual auto admit_n_drop(vector<unique_ptr<Packet>> p) -> vector<unique_ptr<Packet>> = 0;
    virtual auto select_for_processing() -> Packet * = 0;
    virtual void processing_finished(Packet * packet) = 0;
    virtual auto transmit() -> unique_ptr<Packet> = 0;

    virtual auto num_packets_in_buffer() const noexcept -> size_t = 0; 

    virtual auto take() -> vector<unique_ptr<Packet>> = 0;
    virtual auto peek() const -> vector<Packet const *> = 0;

    PolicyInstance() = default;
    PolicyInstance(PolicyInstance const&) = delete;
    PolicyInstance& operator=(PolicyInstance const&) = delete;

    virtual ~PolicyInstance() = default;
};

struct Policy {
    virtual auto name() const -> string = 0;

    virtual auto instantiate(World * world, size_t buffer_size) const 
        -> unique_ptr<PolicyInstance> = 0;

    virtual void to_json(json& j) const = 0;

    virtual ~Policy() = default;
};

inline void to_json(json& j, Policy const& p) {
    p.to_json(j);
}


}

#endif
