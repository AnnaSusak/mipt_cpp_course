#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H
#include <cstddef>
#include <map>
#include <string>
#include "ring_buffer.h"
#include "event_list.h"
#include "event_source.h"
#include "rule_engine.h"
#include <event.h>

namespace nano_edr {
struct WindowEntry {
    Timestamp ts;
    std::string type;
    std::string pid;
};
class Agent: public IEventHandler {
 public:
    static constexpr std::size_t kWindowCapacity = 256;
    explicit Agent(bool quiet);
    void HandleEvent(const Event& event) override;
    void PrintSummary() const;
    void OnSkipped() override;
    void OnBroken() override;
    void OnTick() override;

 private:
    void PrintContext() const;
    RingBuffer<WindowEntry, kWindowCapacity> window_;
    RuleEngine engine_;
    bool quiet_ = false;
    long long all_events_ = 0;
    std::map<std::string, long long> event_types_count_;
    long long skipped_ = 0;
    long long broken_ = 0;
};

}

#endif