#include "utils/packet_sink.h"
#include "utils/drop_target_saver.h"
#include "queue_utils.h"

#include <iostream>
#include <stdexcept>

auto utils::take_packets_and_reset(Queue * queue) -> vector<Packet *> {
    DropTargetSaver drop_target_saver{queue};

    vector<Packet *> result;

    PacketSink sink{back_inserter(result)};
    queue->setDropTarget(&sink);

    Packet * pkt;
    while ((pkt = queue->deque()) != nullptr) {
        result.push_back(pkt);
    }

    Tcl::instance().evalf("%s reset", queue->name());

    return result;
}

void utils::init_queue_with(Queue * queue, vector<Packet *> const& packets) {
    // Saving timestamp since it might be changed by enqueue and 
    // used later as an indication of the enque
    // TODO: maybe just do the normal enqueue?
    assert(queue->length() == 0);
    for (auto packet : packets) { 
        auto const saved_timestamp = HDR_CMN(packet)->timestamp();
        queue->enque(packet);
        HDR_CMN(packet)->ts_ = saved_timestamp;
    }
}
