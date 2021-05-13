#ifndef NS_QUEUE_UTILS_H
#define NS_QUEUE_UTILS_H

#include "queue.h"
#include <vector>

namespace utils {

auto take_packets_and_reset(Queue * queue) -> vector<Packet *>;
void init_queue_with(Queue * queue, vector<Packet *> const& packets);

}

#endif // NS_QUEUE_UTILS_H
