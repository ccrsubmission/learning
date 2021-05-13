#ifndef STATCOLLECTOR_HH_
#define STATCOLLECTOR_HH_

#include <tcl.h>
#include <config.h>

class StatCollector {
 public:
  StatCollector();
  void add_sample(Packet *pkt);
  void output_stats(Tcl_Channel channel, double on_duration,
                    uint32_t flow_id,
                    nsaddr_t daddr, nsaddr_t dport,
                    uint32_t payload_size);
  void reset();
 private:
  uint32_t num_rtt_samples_;
  double   minimum_rtt_;
  double   cumulative_rtt_;
  uint32_t cumulative_pkts_;
  int32_t  last_ack_;
};

#endif // STATCOLLECTOR_HH_
