#ifndef PACKET_H
#define PACKET_H

#include <schad/common.h>

namespace schad {

struct Packet {
private:
    struct tag;

public:
    Packet(tag, uint32_t id, uint32_t time_of_arrival,
           uint32_t initial_processing, double value, optional<uint32_t> slack) 
        : id_{id}, time_of_arrival_{time_of_arrival}, 
          initial_processing_{initial_processing}, value_{value}, slack_{slack},
          remaining_processing_{initial_processing} {
    }

    friend class PacketBuilder;

    auto id() const {
        return id_;
    }

    auto initial_processing() const {
        return initial_processing_;
    }

    auto time_of_arrival() const {
        return time_of_arrival_;
    }

    auto value() const {
        return value_;
    }

    auto remaining_processing() const {
        return remaining_processing_; 
    }

    auto process() {
        assert(remaining_processing() > 0);
        remaining_processing_--;
    }

    auto slack() const {
        return slack_;
    }

    auto clone() -> unique_ptr<Packet> {
        return make_unique<Packet>(tag{}, id(), time_of_arrival(),
           initial_processing(), value(), slack()); 
    }

private:
    struct tag {};

private:
    uint32_t const id_;
    uint32_t const time_of_arrival_;

    uint32_t const initial_processing_;
    double const value_;
    optional<uint32_t> const slack_;

    uint32_t remaining_processing_; 
};


inline bool is_ready(Packet const& packet) {
    return packet.remaining_processing() == 0;
}

inline auto deadline(Packet const& packet) {
    assert(packet.slack().has_value());
    return packet.time_of_arrival() + packet.initial_processing() 
        + *packet.slack() + 1 - packet.remaining_processing();
}

inline bool is_expired(uint32_t time, Packet const& packet) {
    return packet.slack().has_value() && deadline(packet) <= time;
}

}

#endif
