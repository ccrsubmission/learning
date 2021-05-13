#ifndef NS_REWARD_H
#define NS_REWARD_H

#include "packet.h"
#include "learning_common.h"
#include <memory>
#include <vector>


class Reward : public TclObject {
public:
    virtual void note_arrival(Packet const *p);
    virtual void note_drop(Packet const *p);
    virtual void note_transmission(Packet const * p) = 0;

    virtual auto get_value() const -> double = 0;

    virtual void reset(vector<Packet const*> packets) = 0;

    virtual void write_stats(ostream& out, size_t interval_idx) const;

    virtual auto clone() const -> unique_ptr<Reward> = 0;
};


#endif //NS_REWARD_H
