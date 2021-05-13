#ifndef NS_DROP_PROXY_H
#define NS_DROP_PROXY_H

#include "learning_common.h"
#include "queue.h"

class LearningQueue;

namespace utils {

class DropProxy : public NsObject {
public:
    explicit DropProxy(LearningQueue * queue);

    void recv(Packet* p, const char *s) override;
    void recv(Packet* p, Handler * callback) override;

private:
    LearningQueue * queue_;
};

}

#endif // NS_DROP_PROXY_H
