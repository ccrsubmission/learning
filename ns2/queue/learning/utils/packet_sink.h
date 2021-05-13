#ifndef PACKET_SINK_H
#define PACKET_SINK_H

#include "packet.h"

namespace utils {

template<class OutputIterator>
class PacketSink : public NsObject {
public:
    explicit PacketSink(OutputIterator out) : out_{out} { }

    PacketSink(PacketSink const&) = delete;
    PacketSink& operator=(PacketSink const&) = delete;

    void recv(Packet* p, const char *s) override {
        (out_++) = p;
    }

    void recv(Packet* p, Handler * callback) override {
        (out_++) = p;
    }

private:
    OutputIterator out_;
};

}

#endif // PACKET_SINK_H
