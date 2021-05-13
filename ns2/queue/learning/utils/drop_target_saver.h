#ifndef NS_DROP_TARGET_SAVER_H
#define NS_DROP_TARGET_SAVER_H

#include "queue.h"

namespace utils {

struct DropTargetSaver {
public:    
    explicit DropTargetSaver(Queue * queue) 
        : queue_{queue}, drop_target_{queue->getDropTarget()} {
    }

    ~DropTargetSaver() {
        queue_->setDropTarget(drop_target_);
    }

private:
    Queue * const queue_;
    NsObject * const drop_target_;
};

}

#endif // NS_DROP_TARGET_SAVER_H
