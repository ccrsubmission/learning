#include "packet_builder.h"

namespace schad {

auto PacketBuilder::build() -> unique_ptr<Packet> {
    if (!time_of_arrival_.has_value()) {
        throw invalid_packet_parameters("Time of arrival must be specified");
    }
    return make_unique<Packet>(Packet::tag{},
        next_id(), time_of_arrival_.value(),
        initial_processing_, value_, slack_
    );
}

}


