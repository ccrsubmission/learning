#include <cmath>
#include <assert.h>
#include "packet.h"
#include "statcollector.hh"
#include "tcp.h"
#include "flags.h"
#include <stdexcept>
#include <string>

StatCollector::StatCollector()
    : 
      num_rtt_samples_(0),
      minimum_rtt_(-1),
      cumulative_rtt_(0.0),
      cumulative_pkts_(0),
      last_ack_(-1) {}
 
void StatCollector::add_sample(Packet* pkt) {
  /* Get headers */
  hdr_cmn *cmn_hdr = hdr_cmn::access(pkt);
  hdr_tcp *tcph = hdr_tcp::access(pkt);
  hdr_flags *fh = hdr_flags::access(pkt);

  /* Assert that packet is an ACK */
  assert(cmn_hdr->ptype() == PT_ACK || cmn_hdr->ptype() == PT_TCP);

  /* Get size of packet, iff it's a new ack */
  int32_t current_ack = tcph->seqno();
  assert (current_ack >= last_ack_); /* ACKs are cumulative */
  if (current_ack > last_ack_) {
    cumulative_pkts_ += (current_ack - last_ack_);
    last_ack_ = current_ack;
  }

  /* Get RTT */
  if (tcph->ts_echo() > 0.0) {
    double const rtt = Scheduler::instance().clock() - tcph->ts_echo();
    if (minimum_rtt_ < 0 || rtt < minimum_rtt_) {
        minimum_rtt_ = rtt;
    }
    cumulative_rtt_ += rtt;
    num_rtt_samples_++;
  }
}

void StatCollector::output_stats(Tcl_Channel channel, double on_duration,
                                 uint32_t flow_id,
                                 nsaddr_t daddr, nsaddr_t dport,
                                 uint32_t payload_size) {
  assert(on_duration != 0);
  char wrk[512];
  snprintf(wrk, sizeof(wrk) / sizeof(char),
          "%u: daddr=%d, dport=%d,"
          " tp=%f mbps, del=%f ms, mindel=%f ms, on=%f secs, samples=%d, inorder=%d\n",
          flow_id, daddr, dport,
          (cumulative_pkts_ * 8.0 * payload_size)/ (1.0e6 * on_duration),
          num_rtt_samples_ 
            ? (cumulative_rtt_ * 1000.0) / num_rtt_samples_ : INFINITY,
          (minimum_rtt_ * 1000.0),
          on_duration,
          num_rtt_samples_,
          cumulative_pkts_);
  if (channel) {
    Tcl_Write(channel, wrk, -1);
  } else {
    fprintf(stderr, "%s", wrk);
  }
}

void StatCollector::reset() {
    num_rtt_samples_ = 0;
    cumulative_pkts_ = 0;
    cumulative_rtt_ = 0;
    last_ack_ = -1;
}
