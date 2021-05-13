#ifndef DETAIL_PQ_H
#define DETAIL_PQ_H

#include <schad/policy/policy.h>

#include <set>
#include <queue>
#include <type_traits>

namespace {

using namespace schad;

template<class Priority> 
class PQPolicyInstance : public PolicyInstance {
public:
    PQPolicyInstance(World * world, size_t buffer_size, Priority prio) 
        : world_{world}, buffer_size_{buffer_size}, prio_{prio}, queue_{} {
    }

    auto admit_n_drop(vector<unique_ptr<Packet>> ps)
            -> vector<unique_ptr<Packet>> override {
        admit(std::move(ps));
        return drop();
    }

    auto transmit() -> unique_ptr<Packet> override {
        if (!queue_.empty() && is_ready(*queue_.front())) {
            auto result = std::move(queue_.front());
            queue_.pop_front();
            return std::move(result);
        }

        return nullptr;
    }

    auto select_for_processing() -> Packet * override {
        if (queue_.empty()) {
            return nullptr;
        }
        auto best_it = std::min_element(
                begin(queue_), end(queue_), create_comparator());
        swap(queue_.front(), *best_it);
        return queue_.front().get();
    }

    void processing_finished(Packet * packet) override {
        assert(queue_.front().get() == packet);
    }

    auto num_packets_in_buffer() const noexcept -> size_t override  {
        return queue_.size();
    }

    auto take() -> vector<unique_ptr<Packet>> override {
        vector<unique_ptr<Packet>> result(queue_.size());
        std::move(begin(queue_), end(queue_), begin(result));
        queue_.clear();
        return result;
    }

    auto peek() const -> vector<Packet const *> override {
        vector<Packet const *> result(queue_.size());
        std::transform(begin(queue_), end(queue_), begin(result),
                [] (auto const& p) { return p.get(); });
        return result;
    }

private:
    auto create_comparator() {
        return [t=world_->current_time(), prio=prio_] 
            (auto const& a, auto const& b) {
                return prio(t, *a) < prio(t, *b);
        };
    }

    void admit(vector<unique_ptr<Packet>> ps) {
        queue_.resize(queue_.size() + ps.size());
        std::move(begin(ps), end(ps), rbegin(queue_));
    }

    auto drop() -> vector<unique_ptr<Packet>> {
        auto end_it = std::partition(begin(queue_), end(queue_), 
            [t=world_->current_time()](auto const& p) { return !is_expired(t, *p); });

        if (std::distance(begin(queue_), end_it) > int(buffer_size_)) {
            std::nth_element(begin(queue_), begin(queue_) + buffer_size_, 
                    end_it, create_comparator());
            end_it = begin(queue_) + buffer_size_;
        }

        vector<unique_ptr<Packet>> result(std::distance(end_it, end(queue_)));
        std::move(end_it, end(queue_), begin(result));
        queue_.erase(end_it, end(queue_));
        return result;
    }

private:
    World * const world_;
    size_t const buffer_size_;

    Priority prio_;
    std::deque<unique_ptr<Packet>> queue_;
};

}

namespace schad::detail {

template<class Priority> 
class PQPolicy : public Policy {
public:
    PQPolicy(std::string name, Priority prio) 
        : name_{std::move(name)}, prio_{prio} {
    }

    auto name() const -> string override {
        return name_;
    }

    auto instantiate(World * world, size_t buffer_size) const 
            -> unique_ptr<PolicyInstance> override {
        auto constexpr const is_packet_prio = std::is_invocable_v<Priority, Packet const&>;
        auto constexpr const is_time_packet_prio = std::is_invocable_v<Priority, uint32_t, Packet const&>;
        static_assert(is_packet_prio || is_time_packet_prio,
            "prio must be either p(Packet const&) or p(uint32_t time, Packeg const&)"
        );
        if constexpr (is_packet_prio) {
            auto const new_prio = [prio=prio_] (auto t, auto const& x) {
                    return make_tuple(is_expired(t, x) ? 1 : 0, prio(x), x.id());
            };
            return make_unique<PQPolicyInstance<decltype(new_prio)>>(
                world, buffer_size, new_prio
            );
        } else {
            auto const new_prio = [prio=prio_] (auto t, auto const& x) {
                return make_tuple(is_expired(t, x) ? 1 : 0, prio(t, x), x.id());
            };
            return make_unique<PQPolicyInstance<decltype(new_prio)>>(
                world, buffer_size, new_prio
            );
        }     
    }


    void to_json(json& j) const override {
        j = {{"type", name()}};
    }

private:
    string const name_;
    Priority prio_;
};

}

#endif // DETAIL_PQ_H
