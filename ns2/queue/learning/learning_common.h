#ifndef LEARNING_COMMON_H
#define LEARNING_COMMON_H

#include "packet.h"
#include "ip.h"

#include <type_traits>
#include <functional>

using FlowID = std::decay_t<decltype(declval<hdr_ip *>()->flowid())>;

template<> 
struct std::hash<Packet *> {
    size_t operator()(Packet * const packet) const {
        return HDR_CMN(packet)->uid();
    }
};

#endif
