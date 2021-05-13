#include "flow_statistics.h"

#include "tcp.h"

FlowStatistic::FlowStatistic()
    :   flow_start_time_{Scheduler::instance().clock()}
    ,   num_packets_buffered_{0}
    ,   num_packets_transmitted_{0}
    ,   avg_delay_{0}
    ,   total_bytes_transmitted_{0}
    ,   last_transmission_time_{std::nullopt} {
}

void FlowStatistic::note_arrival(Packet const * packet) {
    num_packets_buffered_++;
}

void FlowStatistic::note_drop(Packet const * packet) {
    num_packets_buffered_--;
}

void FlowStatistic::note_transmission(Packet const * packet) {
    auto const cmn_hdr = hdr_cmn::access(packet);
    auto const tcp_hdr = hdr_tcp::access(packet);
    auto const now = Scheduler::instance().clock();
    auto const delay = now - cmn_hdr->timestamp();

    num_packets_buffered_--;
    num_packets_transmitted_++;
    total_bytes_transmitted_ += cmn_hdr->size() - tcp_hdr->hlen();
    avg_delay_ += (delay - avg_delay_) / num_packets_transmitted_;
    last_transmission_time_ = now;
}

auto FlowStatistic::get_flow_end_time() const -> double {
    if (num_packets_buffered_ > 0) {
        return Scheduler::instance().clock();
    } else {
        return *last_transmission_time_;  // must not be none
    }
}

auto FlowStatistic::get_active_interval(double interval) const
        -> double {
    if (get_flow_end_time() == flow_start_time_) {
        // TODO: Is this branch possible?
        return interval;
    } else {
        return get_flow_end_time() - flow_start_time_;
    }
}

auto FlowStatistic::get_avg_delay() const -> double {
    return avg_delay_;
}

auto FlowStatistic::get_total_bytes_transmitted() const -> size_t {
    return total_bytes_transmitted_;
}

auto FlowStatistic::get_throughput(double interval) const
        -> double {
    return get_total_bytes_transmitted() / get_active_interval(interval);
}
