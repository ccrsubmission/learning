#ifndef PACKET_BUILDER_H
#define PACKET_BUILDER_H

#include <schad/packet/packet.h>

namespace schad {

struct invalid_packet_parameters : std::invalid_argument {
    using std::invalid_argument::invalid_argument;
};

struct PacketBuilder {
    static auto create() -> PacketBuilder {
        return PacketBuilder();
    }

    auto& set_value(double value) {
        value_ = value;
        return *this;
    }

    auto& set_initial_processing(uint32_t initial_processing) {
        initial_processing_ = initial_processing;
        return *this;
    }

    auto& set_time_of_arrival(uint32_t time_of_arrival) {
        time_of_arrival_ = time_of_arrival;
        return *this;
    }

    auto& set_slack(uint32_t slack) {
        slack_ = slack;
        return *this;
    }

    auto build() -> unique_ptr<Packet>;

private:
    PacketBuilder()
        : time_of_arrival_{}, initial_processing_{1}, value_{1.0} {
    }

    static uint32_t next_id() {
        static uint32_t next_unused_id = 0;
        return next_unused_id++;
    }

private:
    optional<uint32_t> time_of_arrival_;

    uint32_t initial_processing_;
    double value_;
    optional<uint32_t> slack_;
};

}

#endif
