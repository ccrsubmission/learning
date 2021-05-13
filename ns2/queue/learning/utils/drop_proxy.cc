#include "drop_proxy.h"
#include "learning_queue.h"

namespace utils {

DropProxy::DropProxy(LearningQueue * queue) : queue_{queue} {}

void DropProxy::recv(Packet* p, const char *s) {
    queue_->drop(p, s);
}

void DropProxy::recv(Packet* p, Handler * callback) {
    queue_->drop(p);
}

}
