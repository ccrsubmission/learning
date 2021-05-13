#ifndef FLOW_STATISTICS_H
#define FLOW_STATISTICS_H

#include "packet.h"

#include <optional>

struct FlowStatistic {
    FlowStatistic();

    void note_arrival(Packet const * packet);
    void note_drop(Packet const * packet);
    void note_transmission(Packet const * packet);

    auto get_total_bytes_transmitted() const -> size_t;
    auto get_active_interval(double interval) const -> double;
    auto get_throughput(double interval) const -> double;
    auto get_avg_delay() const -> double;

private:
    auto get_flow_end_time() const -> double;

private:
    double const flow_start_time_;

    size_t num_packets_buffered_;
    size_t num_packets_transmitted_;
    double avg_delay_;
    double total_bytes_transmitted_;
    std::optional<double> last_transmission_time_;
};

#endif // FLOW_STATISTICS_H
