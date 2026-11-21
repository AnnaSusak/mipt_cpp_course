#include "detection.h"
#include <format>
namespace nano_edr{
  std::string ToString(const Detection& detection){
    std::string detect = std::format("[DETECT] {}  {}  ts={} pid={}",
        SeverityName(detection.severity),
        detection.rule,
        detection.ts.ms,
        detection.pid);
    if (!detection.chain.empty()) {
        detect += std::format(" chain={}", detection.chain);
    }
    return detect;
  }
}