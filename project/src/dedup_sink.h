
#ifndef NANO_EDR_DEDUP_SINK_H
#define NANO_EDR_DEDUP_SINK_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "alert_sink.h"
#include "detection.h"

namespace nano_edr {

class DedupSink {
 public:
    DedupSink(AlertSink inner, uint64_t period_ms, std::size_t* suppressed);

    void operator()(const Detection& detection);
private:
    AlertSink inner_;
    uint64_t period_ms_;
    std::size_t* suppressed_;
    std::vector<std::string> rule_names_;
    std::vector<std::string> pids_;
    std::vector<uint64_t> first_ts_;
};

}

#endif
